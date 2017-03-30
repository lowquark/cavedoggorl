
#include "draw.hpp"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

namespace draw {
  Vec2i camera_pos;

  void draw_wall(Vec2i pos) {
    float x = (pos.x - camera_pos.x) * TILE_WIDTH;
    float y = (pos.y - camera_pos.y) * TILE_HEIGHT;

    glTranslatef(x, y, 0.0f);

    glColor3f(WALL_COLOR.r, WALL_COLOR.g, WALL_COLOR.b);
    glBegin(GL_LINES);
      glVertex2f(-WALL_WIDTH/2, -WALL_HEIGHT/2);
      glVertex2f( WALL_WIDTH/2, -WALL_HEIGHT/2);

      glVertex2f( WALL_WIDTH/2, -WALL_HEIGHT/2);
      glVertex2f( WALL_WIDTH/2,  WALL_HEIGHT/2);

      glVertex2f( WALL_WIDTH/2,  WALL_HEIGHT/2);
      glVertex2f(-WALL_WIDTH/2,  WALL_HEIGHT/2);

      glVertex2f(-WALL_WIDTH/2,  WALL_HEIGHT/2);
      glVertex2f(-WALL_WIDTH/2, -WALL_HEIGHT/2);
    glEnd();

    glTranslatef(-x, -y, 0.0f);
  }
  void draw_floor(Vec2i pos) {
    float x = (pos.x - camera_pos.x) * TILE_WIDTH;
    float y = (pos.y - camera_pos.y) * TILE_HEIGHT;

    glTranslatef(x, y, 0.0f);

    glColor3f(FLOOR_COLOR.r, FLOOR_COLOR.g, FLOOR_COLOR.b);
    glBegin(GL_QUADS);
      glVertex2f(-FLOOR_DOT_WIDTH/2, -FLOOR_DOT_HEIGHT/2);
      glVertex2f( FLOOR_DOT_WIDTH/2, -FLOOR_DOT_HEIGHT/2);
      glVertex2f( FLOOR_DOT_WIDTH/2,  FLOOR_DOT_HEIGHT/2);
      glVertex2f(-FLOOR_DOT_WIDTH/2,  FLOOR_DOT_HEIGHT/2);
    glEnd();

    glTranslatef(-x, -y, 0.0f);
  }

  void draw_agent(Vec2i pos, const Color & color) {
    float x = (pos.x - camera_pos.x) * TILE_WIDTH;
    float y = (pos.y - camera_pos.y) * TILE_HEIGHT;

    glTranslatef(x, y, 0.0f);

    glColor3f(color.r, color.g, color.b);
    glBegin(GL_QUADS);
      glVertex2f(-AGENT_WIDTH/2, -AGENT_HEIGHT/2);
      glVertex2f( AGENT_WIDTH/2, -AGENT_HEIGHT/2);
      glVertex2f( AGENT_WIDTH/2,  AGENT_HEIGHT/2);
      glVertex2f(-AGENT_WIDTH/2,  AGENT_HEIGHT/2);
    glEnd();

    glTranslatef(-x, -y, 0.0f);
  }
  void draw_path(const std::vector<Vec2i> & path, const Color & color) {
    if(path.size() > 0) {
      glColor3f(color.r, color.g, color.b);
      glBegin(GL_LINES);
        for(unsigned int i = 0 ; i < path.size() - 1 ; i ++) {
          float x0 = (path[i    ].x - camera_pos.x) * TILE_WIDTH;
          float y0 = (path[i    ].y - camera_pos.y) * TILE_HEIGHT;
          float x1 = (path[i + 1].x - camera_pos.x) * TILE_WIDTH;
          float y1 = (path[i + 1].y - camera_pos.y) * TILE_HEIGHT;

          glVertex2f(x0, y0);
          glVertex2f(x1, y1);
        }
      glEnd();
    }
  }
}

