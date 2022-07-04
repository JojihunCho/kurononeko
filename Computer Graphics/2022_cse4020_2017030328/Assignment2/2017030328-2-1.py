import glfw
from OpenGL.GL import *
import numpy as np


def key_callback(window, key, scancode, action, mods):
    global type
    if key==glfw.KEY_1:
        if action==glfw.PRESS:
            type = GL_POINTS
    if key==glfw.KEY_2:
        if action==glfw.PRESS:
            type = GL_LINES
    if key==glfw.KEY_3:
        if action==glfw.PRESS:
            type = GL_LINE_STRIP
    if key==glfw.KEY_4:
        if action==glfw.PRESS:
            type = GL_LINE_LOOP
    if key==glfw.KEY_5:
        if action==glfw.PRESS:
            type = GL_TRIANGLES
    if key==glfw.KEY_6:
        if action==glfw.PRESS:
            type = GL_TRIANGLE_STRIP
    if key==glfw.KEY_7:
        if action==glfw.PRESS:
            type = GL_TRIANGLE_FAN
    if key==glfw.KEY_8:
        if action==glfw.PRESS:
            type = GL_QUADS
    if key==glfw.KEY_9:
        if action==glfw.PRESS:
            type = GL_QUAD_STRIP
    if key==glfw.KEY_0:
        if action==glfw.PRESS:
            type = GL_POLYGON


def render():
    global type
    glClear(GL_COLOR_BUFFER_BIT)
    glLoadIdentity()
    glBegin(type)
    x = np.linspace(0, 360 * 3.14159265 / 180, 13)
    for i in x:
        glVertex2f(np.cos(i), np.sin(i))
    glEnd()


def main():
    global type
    type = GL_POINTS
    if not glfw.init():
        return

    window = glfw.create_window(480,480,"2017030328-2-1", None, None)
    if not window:
        glfw.terminate()
        return

    glfw.set_key_callback(window, key_callback)

    glfw.make_context_current(window)

    while not glfw.window_should_close(window):

        glfw.poll_events()

        render()

        glfw.swap_buffers(window)
    
    glfw.terminate()


if __name__ == "__main__":
    main()