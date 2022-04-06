import glfw
from OpenGL.GL import *
import numpy as np

T = np.array( [[1., 0., 0.],
              [0., 1., 0.],
              [0., 0., 1.]] )
        
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


def key_callback(window, key, scancode, action, mods):
    global T

    if key==glfw.KEY_Q and action==glfw.PRESS:

        T2 = np.array([[1., 0., -0.1],
                      [0., 1., 0.],
                      [0., 0., 1.]] )
        T = T2@T
        
    if key==glfw.KEY_E and action==glfw.PRESS:
        T2 = np.array([[1., 0., 0.1],
                      [0., 1., 0.],
                      [0., 0., 1.]] )
        T = T2@T
        
    if key==glfw.KEY_A and action==glfw.PRESS:
        th = np.radians(10)

        sin = np.sin(th)
        cos = np.cos(th)
        
        T2 = [[cos, -sin, 0.],
             [sin, cos, 0.],
             [0., 0., 1.]]
        T = T@T2
               

    if key==glfw.KEY_D and action==glfw.PRESS:
        th = np.radians(-10)

        sin = np.sin(th)
        cos = np.cos(th)
                     
        T2 = [[cos, -sin, 0.],
             [sin, cos, 0.],
             [0., 0., 1.]]
        
        T = T@T2
        
    if key==glfw.KEY_1 and action==glfw.PRESS:
        T = np.array( [[1., 0., 0.],
                      [0., 1., 0.],
                      [0., 0., 1.]] )
        
    if key==glfw.KEY_W and action==glfw.PRESS:
        T2 = [[0.9, 0., 0.],
             [0., 1., 0.],
             [0., 0., 1.]]
        T = T2@T
        
    if key==glfw.KEY_S and action==glfw.PRESS:
        th = np.radians(10)

        sin = np.sin(th)
        cos = np.cos(th)
        
        T2 = [[cos, -sin, 0.],
             [sin, cos, 0.],
             [0., 0., 1.]]
        T = T2@T

        
def main():
        if not glfw.init():
                return

        window = glfw.create_window(640, 640, "2018009234", None, None)

        if not window:
            glfw.terminate()
            return

        glfw.set_key_callback(window, key_callback)

        glfw.make_context_current(window)

        while not glfw.window_should_close(window):
            glfw.poll_events()
            render(T)
            glfw.swap_buffers(window)
        glfw.terminate()

if __name__ == "__main__":
    main()
