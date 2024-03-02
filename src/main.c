#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

#include "utils.h"
#include "graph.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define NODE_WIDTH 60
#define NODE_HEIGHT 60

#define NODE_BORDER_WIDTH 3

char *exec_name;

void die(int rc, char *msg) {
    fprintf(stderr, "\033[91m%s [fatal error]: %s\033[0m\n", exec_name, msg);
    exit(rc);
}

void print_help(int rc) {
    fprintf(stderr, "USAGE: %s (wow)\n", exec_name);
    exit(rc);
}

void *xmalloc(size_t cnt) {
    void *ret = malloc(cnt);
    if (ret == NULL)
        die(1, "malloc failed");
    return ret;
}

int get_node_hit(node_t **nodes, int nodes_len, int x, int y) {
    for (int i = nodes_len - 1; i >= 0; i--)
        if (x >= nodes[i]->x && x < nodes[i]->x + nodes[i]->w && y >= nodes[i]->y && y < nodes[i]->y + nodes[i]->h)
            return i;
    return -1;
}

float get_angle(int x1, int x2, int y1, int y2) {
    float x_v = x2 - x1, y_v = y2 - y1;
    float mag = sqrt(x_v * x_v + y_v * y_v);
    float x_n = x_v / mag, y_n = y_v / mag;

    float theta = acos(x_n);
    if (y_n < 0)
        return 2 * PI - theta;
    return theta;
}

int max(int a, int b) {
    return a >= b ? a : b;
}

int min(int a, int b) {
    return a <= b ? a : b;
}

void node_link_points(Vector2 n1_size, node_t *n1, Vector2 n2_size, node_t *n2, Vector2 points[4]) {

    Vector2 n_faces[] = {
        {.x = n1->x + n1_size.x / 2.0f, .y = n1->y}, // T
        {.x = n1->x + n1_size.x, .y = n1->y + n1_size.y / 2.0f}, // R
        {.x = n1->x + n1_size.x / 2.0f, .y = n1->y + n1_size.y}, // B
        {.x = n1->x, .y = n1->y + n1_size.y / 2.0f}, // L
    };
    Vector2 n2_faces[] = {
        {.x = n2->x + n2_size.x / 2.0f, .y = n2->y}, // T
        {.x = n2->x + n2_size.x, .y = n2->y + n2_size.y / 2.0f}, // R
        {.x = n2->x + n2_size.x / 2.0f, .y = n2->y + n2_size.y}, // B
        {.x = n2->x, .y = n2->y + n2_size.y / 2.0f}, // L
    };

    // Find minimum distances
    int min_n = 0;
    int min_n2 = 0;
    float min_dist = Vector2Distance(n_faces[0], n2_faces[0]);
    for (int k = 0; k < 4; k++) {
        for (int r = 0; r < 4; r++) {
            float dist = Vector2Distance(n_faces[k], n2_faces[r]);
            if (dist < min_dist) {
                min_n = k;
                min_n2 = r;
                min_dist = dist;
            }
        }
    }

    points[0].x = n1->x + n1_size.x / 2.0f;
    points[0].y = n1->y + n1_size.y / 2.0f;

    points[3].x = n2->x + n2_size.x / 2.0f;
    points[3].y = n2->y + n2_size.y / 2.0f;

    if (min_n % 2 == 0 && min_n2 % 2 == 0) {
        // top/bottom to top/bottom
        points[1].x = points[0].x;
        points[1].y = (points[0].y + points[3].y)/2;
        points[2].x = points[3].x;
        points[2].y = points[1].y;
    }
    else if (min_n % 2 == 1 && min_n2 % 2 == 0) {
        // left/right for n to top/bottom for n2
        points[1].x = 0.75 * points[3].x + 0.25 * points[0].x;
        points[1].y = points[0].y;
        points[2].x = points[3].x;
        points[2].y = 0.75 * points[0].y + 0.25 * points[3].y;
    } 
    else if (min_n % 2 == 0 && min_n2 % 2 == 1) {
        // left/right for n2 to top/bottom for n
        points[1].x = points[0].x;
        points[1].y = 0.75 * points[3].y + 0.25 * points[0].y;
        points[2].x = 0.75 * points[0].x + 0.25 * points[3].x;
        points[2].y = points[3].y;
    } 
    else if (min_n % 2 == 1 && min_n2 % 2 == 1) {
        // left/right to left/right
        points[1].x = (points[0].x + points[3].x)/2;
        points[1].y = points[0].y;
        points[2].x = points[1].x;
        points[2].y = points[3].y;
    }
    else {
        die(1, "Never should be reached");
    }
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char **argv) {
    exec_name = argv[0];

    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
            print_help(0);

        if (strcmp(argv[1], "wow") == 0)
            die(1, "wow!");
    }

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Testosterone");

    Font default_font = GetFontDefault();

    SetTargetFPS(60);

    int node_cnt = 0;
    node_t *nodes[100];
    char txtbuf[1024];

    bool clicked_node = false;
    int clicked_node_u = 0, clicked_node_v = 0;
    int click_start_frame = -1, click_start_x = 0, click_start_y = 0;

    int selected_node = -1;

    int frame_cnt = 0;

    for (; !WindowShouldClose(); frame_cnt++) {
        // Handling inputs

        int mx = GetMouseX(), my = GetMouseY();

        // Add a node when "N" is pressed
        if (IsKeyPressed(KEY_N)) {
            nodes[node_cnt] = create_node();
            snprintf(txtbuf, sizeof(txtbuf), "Node %d", node_cnt);
            write_node_text(nodes[node_cnt], txtbuf);
            nodes[node_cnt]->w = NODE_WIDTH;
            nodes[node_cnt]->h = NODE_HEIGHT;
            nodes[node_cnt]->x = GetScreenWidth() / 2 - 30;
            nodes[node_cnt]->y = GetScreenHeight() / 2 - 30;
            node_cnt++;
        }

        // Delete a node when "D" is pressed
        if (IsKeyPressed(KEY_D)) {
            int nh = get_node_hit(nodes, node_cnt, mx, my);
            if (nh >= 0) {
                for (int i = 0; i < node_cnt; i++)
                    if (i != nh)
                        unlink_nodes(nodes[i], nodes[nh]);
                destroy_node(nodes[nh]);
                if (selected_node > nh)
                    selected_node--;
                else if (selected_node == nh)
                    selected_node = -1;
                // Shift all of the nodes after the hit node back by one
                for (int i = nh; i < node_cnt - 1; i++)
                    nodes[nh] = nodes[nh+1];
                node_cnt--;
            }
        }

        // Select a node to direct a link to with right mouse
        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && selected_node >= 0) {
            int nh = get_node_hit(nodes, node_cnt, mx, my);
            if (nh >= 0 && nh != selected_node) {
                link_nodes(nodes[selected_node], nodes[nh]);
                selected_node = -1;
            }
        }

        // Logic for dragging and selecting nodes with left mouse
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (frame_cnt - click_start_frame <= 10) {
                // Short press
                if (selected_node == node_cnt - 1)
                    // Pressed on the same node. Deselect node
                    selected_node = -1;
                else
                    selected_node = node_cnt - 1;
            }

            clicked_node = false;
            click_start_frame = -1;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (click_start_frame < 0) {
                click_start_frame = frame_cnt;
                click_start_x = mx;
                click_start_x = my;
            }

            if (!clicked_node) {
                int ind = get_node_hit(nodes, node_cnt, mx, my);
                if (ind >= 0) {
                    clicked_node = true;

                    // Get the relative coordinates of the mouse on the selected node. The "UV" coordinates
                    clicked_node_u = mx - nodes[ind]->x;
                    clicked_node_v = my - nodes[ind]->y;

                    // Swap last node and this node, then move all of the nodes back
                    node_t *tmp = nodes[ind];
                    for (int i = ind; i < node_cnt - 1; i++) {
                        if (i+1 == selected_node)
                            selected_node--;
                        nodes[i] = nodes[i+1];
                    }
                    nodes[node_cnt-1] = tmp;
                }
            }

            if ((frame_cnt - click_start_frame > 30 || 
                (abs(click_start_x - mx) > 20 || abs(click_start_y - my) > 20)) 
                && clicked_node) 
            {
                node_t *n = nodes[node_cnt-1];
                n->x = mx - clicked_node_u;
                n->y = my - clicked_node_v;
            }
        }

        // Drawing

        BeginDrawing();

        {
            ClearBackground(BLACK);
            for (int i = 0; i < node_cnt; i++) {
                node_t *n = nodes[i];
                Vector2 text_size = MeasureTextEx(default_font, n->text, 11, 3);

                int width = max(text_size.x, n->w);
                int height = max(text_size.y, n->h);

                // Draw node links
                Vector2 points[4];
                for (node_list_t *j = n->connecting_nodes; j != NULL; j = j->next) {
                    node_link_points(n, j->node, points);
                    DrawSplineBezierCubic(points, 4, 5, WHITE);
                }

                // Draw node body
                Rectangle outer = {
                    .x = n->x,
                    .y = n->y,
                    .width = n_size.x,
                    .height = n_size.y,
                };
                if (selected_node == i)
                    DrawRectangleRounded(outer, 0.2f, 3, RED);
                else
                    DrawRectangleRounded(outer, 0.2f, 3, GRAY);
                Rectangle inner = {
                    .x = n->x + NODE_BORDER_WIDTH,
                    .y = n->y + NODE_BORDER_WIDTH,
                    .width = n_size.x - NODE_BORDER_WIDTH * 2,
                    .height = n_size.y - NODE_BORDER_WIDTH * 2,
                };
                DrawRectangleRounded(inner, 0.2f, 3, WHITE);

                DrawText(n->text, n->x + n_size.x / 2 - text_size.x / 2, 
                    n->y + n_size.y / 2 - text_size.y / 2, 11, RED);
            }

            DrawFPS(0, 0);
        }

        EndDrawing();
    }

    for (int i = 0; i < node_cnt; i++)
        destroy_node(nodes[i]);

    CloseWindow();
}
