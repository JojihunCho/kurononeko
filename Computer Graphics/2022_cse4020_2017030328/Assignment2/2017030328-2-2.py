import glfw
from OpenGL.GL import *
import numpy as np

def render():
    pass

def render(T):
    glClear(GL_COLOR_BUFFER_BIT)
    glLoadIdentity()
    # draw cooridnate
    glBegin(GL_LINES)
    glColor3ub(255, 0, 0)
    glVertex2fv(np.array([0.,0.]))
    glVertex2fv(np.array([1.,0.]))
    glColor3ub(0, 255, 0)
    glVertex2fv(np.array([0.,0.]))
    glVertex2fv(np.array([0.,1.]))
    glEnd()
    # draw triangle 
    glBegin(GL_TRIANGLES)
    glColor3ub(255, 255, 255)
    glVertex2fv( (T @ np.array([.0,.5,1.]))[:-1] )
    glVertex2fv( (T @ np.array([.0,.0,1.]))[:-1] )
    glVertex2fv( (T @ np.array([.5,.0,1.]))[:-1] )
    glEnd()

def main():
    if not glfw.init():
        return

    window = glfw.create_window(480,480,"2017030328-2-2", None, None)
    if not window:
        glfw.terminate()
        return

    glfw.make_context_current(window)

    theta = 0

    while not glfw.window_should_close(window):
 # Poll events
        glfw.poll_events()
 # Render here, e.g. using pyOpenGL
        T = np.array([[np.sin(theta), -np.cos(theta) ,0.5 * np.sin(theta)], [np.cos(theta), np.sin(theta), 0.5 * np.cos(theta)], [0,0,1]])
        render(T)
 # Swap front and back buffers
        glfw.swap_buffers(window)
        theta = - glfw.get_time()
    glfw.terminate()


if __name__ == "__main__":
    main()
