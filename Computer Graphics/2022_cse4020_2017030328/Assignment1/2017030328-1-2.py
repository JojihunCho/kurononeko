import numpy, OpenGL, glfw
import numpy as np

M = np.array([2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26])

print("A")
print(M)
print("")

M = M.reshape(5, 5)

print("B")
print(M)
print("")

M[0 ][ 0] = 0
M[1 ][ 0] = 0
M[2 ][ 0] = 0
M[3 ][ 0] = 0
M[4 ][ 0] = 0

print("C")
print(M)
print("")

M = M@M

print("D")
print(M)
print("")

B = M[0] * M[0]

print("E")

V = B[0] + B[1] + B[2] + B[3] + B[4]

V = np.sqrt(V)

print(V)