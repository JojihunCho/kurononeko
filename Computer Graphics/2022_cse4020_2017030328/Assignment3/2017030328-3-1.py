import glfw
from OpenGL.GL import *
from OpenGL.GLU import *
import numpy as np

tScale = 1.
tRocal = 0.
tGlobal = 0.
tTrans = 0.


def render(T):
    glClear(GL_COLOR_BUFFER_BIT)
    glLoadIdentity()

    glBegin(GL_LINES)
    glColor3ub(255, 0, 0)
    glVertex2fv(np.array([0.,0.]))
    glVertex2fv(np.array([1.,0.]))
    glColor3ub(0, 255, 0)
    glVertex2fv(np.array([0.,0.]))
    glVertex2fv(np.array([0.,1.]))
    glEnd()
    glBegin(GL_TRIANGLES)
    glColor3ub(255, 255, 255)
    glVertex2fv( (T @ np.array([.0,.5,1.]))[:-1] )
    glVertex2fv( (T @ np.array([.0,.0,1.]))[:-1] )
    glVertex2fv( (T @ np.array([.5,.0,1.]))[:-1] )
    glEnd()


def key_callback(window, key, scancode, action, mods):
    global tScale, tRocal, tGlobal, tTrans
    if action==glfw.PRESS or action==glfw.REPEAT:
        if key==glfw.KEY_1:
            tScale = 1.
            tRocal = 0.
            tGlobal = 0.
            tTrans = 0.
        elif key==glfw.KEY_Q:
            tTrans += -0.1
        elif key==glfw.KEY_E:
            tTrans += 0.1
        elif key==glfw.KEY_A:
            tRocal += np.radians(10)
        elif key==glfw.KEY_D:
            tRocal += np.radians(-10)
        elif key==glfw.KEY_W:
            tScale *= 0.9
        elif key==glfw.KEY_S:
            tGlobal += np.radians(10)
        

def main():
    global tScale, tRocal, tGlobal, tTrans
    if not glfw.init():
        return
    window = glfw.create_window(480,480,'2017030328-2-2()', None,None)
    if not window:
        glfw.terminate()
        return
    glfw.make_context_current(window)
    glfw.set_key_callback(window, key_callback)

    
    while not glfw.window_should_close(window):
        glfw.poll_events()
        rocRoc = np.array([[np.cos(tRocal), -np.sin(tRocal) ,0], [np.sin(tRocal), np.cos(tRocal), 0], [0,0,1]])
        trans = np.array([[1,0,tTrans], [0,1,0], [0,0,1]])
        gloRoc = np.array([[np.cos(tGlobal), -np.sin(tGlobal) ,0], [np.sin(tGlobal), np.cos(tGlobal), 0], [0,0,1]])
        scale = np.array([[tScale,0,0], [0,1,0], [0,0,1]])
        
        
        T = gloRoc @ trans
        T = T @ rocRoc
        T = T @ scale
        render(T)
        glfw.swap_buffers(window)

    glfw.terminate()

if __name__ == "__main__":
    main()
