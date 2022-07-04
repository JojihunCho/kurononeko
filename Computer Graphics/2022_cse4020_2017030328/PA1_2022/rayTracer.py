#!/usr/bin/env python3
# -*- coding: utf-8 -*
# see examples below
# also read all the comments below.

from operator import index
import os
import sys
import pdb  # use pdb.set_trace() for debugging
import code
from unittest import result
#from turtle import width # or use code.interact(local=dict(globals(), **locals()))  for debugging.
import xml.etree.ElementTree as ET
import numpy as np
from PIL import Image 

FARAWAY = sys.maxsize

class Color:
    def __init__(self, R, G, B):
        self.color=np.array([R,G,B]).astype(np.float64)

    # Gamma corrects this color.
    # @param gamma the gamma value to use (2.2 is generally used).
    def gammaCorrect(self, gamma):
        inverseGamma = 1.0 / gamma;
        self.color=np.power(self.color, inverseGamma)

    def toUINT8(self):
        return (np.clip(self.color, 0,1)*255).astype(np.uint8)

class Shader:
    def __init__(self, type):
        self.type = type


class ShaderPhong(Shader):
    def __init__(self, diffuse, specular, exponent):
        self.name = 'ShaderPhong'
        self.diffuse = diffuse
        self.specular = specular
        self.exponent = exponent


class ShaderLambertian(Shader):
    def __init__(self, diffuse):
        self.name = 'ShaderLambertian'
        self.diffuse = diffuse


class Sphere:
    def __init__(self, center, radius, shader):
        self.name = 'Sphere'
        self.center = center
        self.radius = radius
        self.shader = shader


class Box:
    def __init__(self, minPt, maxPt, shader, normals):
        self.name = 'Box'
        self.minPt = minPt
        self.maxPt = maxPt
        self.shader = shader
        self.normals = normals


class View:
    def __init__(self, viewPoint, viewDir, viewUp, viewProjNormal, viewWidth, viewHeight, projDistance, intensity):
        self.viewPoint = viewPoint
        self.viewDir = viewDir
        self.viewUp = viewUp
        self.viewProjNormal = viewProjNormal
        self.viewWidth = viewWidth
        self.viewHeight = viewHeight
        self.projDistance = projDistance
        self.intensity = intensity


class Light:
    def __init__(self, position, intensity):
        self.position = position
        self.intensity = intensity

def raytrace(objects, ray, viewPoint):
    global FARAWAY
    distence = FARAWAY

    index = -1
    nowIndex = 0

    for i in objects:
        if i.name == 'Sphere':

            result, dist = sphereDistence(ray, viewPoint, i.center, i.radius)
            
            if result:
                if distence >= dist:
                    distence = dist
                    index = nowIndex

        elif i.name == 'Box':
            result, min = boxDistence(ray, viewPoint, i.minPt, i.maxPt)

            if result:
                if distence >= min:
                    distence = min
                    index = nowIndex

        nowIndex += 1

    return [distence, index]

def sphereDistence(ray, viewPoint, center, radius):
    result = False
    distence = 0.
    
    a = np.sum(ray * ray)
    b = np.sum((viewPoint - center) * ray)
    c = np.sum((viewPoint - center) ** 2) - radius ** 2

    if b ** 2 - a * c >= 0:
        if -b + np.sqrt(b ** 2 - a * c) >= 0:
            distence = (-b + np.sqrt(b ** 2 - a * c)) / a
            result = True

        if -b - np.sqrt(b ** 2 - a * c) >= 0:
            distence = (-b - np.sqrt(b ** 2 - a * c)) / a
            result = True
    return result, distence

def boxDistence(ray, viewPoint, minPt, maxPt):
    result = True

    txmin = (minPt[0]-viewPoint[0])/ray[0]
    txmax = (maxPt[0]-viewPoint[0])/ray[0]

    if txmin > txmax:
        txmin, txmax = txmax, txmin

    tymin = (minPt[1]-viewPoint[1])/ray[1]
    tymax = (maxPt[1]-viewPoint[1])/ray[1]

    if tymin > tymax:
        tymin, tymax = tymax, tymin

    if txmin > tymax or tymin > txmax:
        result = False

    if tymin > txmin:
        txmin = tymin
    if tymax < txmax:
        txmax = tymax

    tzmin = (minPt[2]-viewPoint[2])/ray[2]
    tzmax = (maxPt[2]-viewPoint[2])/ray[2]

    if tzmin > tzmax:
        tzmin, tzmax = tzmax, tzmin

    if txmin > tzmax or tzmin > txmax:
        result = False

    if tzmin >= txmin:
        txmin = tzmin
    if tzmax < txmax:
        txmax = tzmax

    return result, txmin


def shade(distence, ray, view, objects, index, light):
    #만나는 object가 없으면 [0, 0, 0] 반환
    if index == -1:
        return np.array([0, 0, 0])
    else:
        R = 0
        G = 0
        B = 0
        normal = np.array([0, 0, 0])
        v = -distence*ray

        if objects[index].name == 'Sphere':
            normal = view.viewPoint + distence*ray - objects[index].center

            #if(abs(np.sqrt(np.sum(normal*normal)) - objects[index].r)>0.000001):
            #    print('check', abs(np.sqrt(np.sum(normal*normal)) - objects[index].r))

            normal = normal / np.sqrt(np.sum(normal * normal))

        elif objects[index].name == 'Box':
            point_i = view.viewPoint + distence*ray
            diff = FARAWAY
            i = -1
            cnt = 0

            for normal in objects[index].normals:
                if abs(np.sum(normal[0:3] * point_i)-normal[3]) < diff:
                    diff = abs(np.sum(normal[0:3] * point_i)-normal[3])
                    i = cnt
                cnt = cnt + 1
            normal = objects[index].normals[i][0:3]
            normal = normal / np.sqrt(np.sum(normal * normal))

        for i in light:
            l_i = v + i.position - view.viewPoint
            l_i = l_i / np.sqrt(np.sum(l_i * l_i))
            check = raytrace(objects, -l_i, i.position)

            if check[1] == index:
                if objects[index].shader.name == 'ShaderPhong':
                    v_unit = v / np.sqrt(np.sum(v**2))
                    h = v_unit + l_i
                    h = h / np.sqrt(np.sum(h*h))
                    addColor = shaderPhong(objects[index], i, l_i, normal, h)
                    R += addColor[0]
                    G += addColor[1]
                    B += addColor[2]

                elif objects[index].shader.name == 'ShaderLambertian':
                    addColor = shaderLambertian(objects[index], i, l_i, normal)
                    R += addColor[0]
                    G += addColor[1]
                    B += addColor[2]
        
        res = Color(R, G, B)
        res.gammaCorrect(2.2)
        return res.toUINT8()

def shaderPhong(object, light, lightInput, normal, h):
    R = object.shader.diffuse[0]*max(0,np.dot(normal,lightInput))*light.intensity[0] + object.shader.specular[0] * light.intensity[0] * pow(max(0, np.dot(normal, h)),object.shader.exponent[0])
    G = object.shader.diffuse[1]*max(0,np.dot(normal,lightInput))*light.intensity[1] + object.shader.specular[1] * light.intensity[1] * pow(max(0, np.dot(normal, h)),object.shader.exponent[0])
    B = object.shader.diffuse[2]*max(0,np.dot(normal,lightInput))*light.intensity[2] + object.shader.specular[2] * light.intensity[2] * pow(max(0, np.dot(normal, h)),object.shader.exponent[0])
    return R, G, B

def shaderLambertian(object, light, lightInput, normal):
    R = object.shader.diffuse[0] * light.intensity[0] * max(0, np.dot(lightInput, normal))
    G = object.shader.diffuse[1] * light.intensity[1] * max(0, np.dot(lightInput, normal))
    B = object.shader.diffuse[2] * light.intensity[2] * max(0, np.dot(lightInput, normal))
    return R, G, B

def getNormal(point_x, point_y, point_z):
    dir = np.cross((point_y-point_x), (point_z-point_x))
    d = np.sum(dir*point_z)
    return np.array([dir[0], dir[1], dir[2], d])

def makeCubeNormal(minPt, maxPt):
    normals = []

    point_a = np.array([minPt[0], minPt[1], maxPt[2]])
    point_b = np.array([minPt[0], maxPt[1], minPt[2]])
    point_c = np.array([maxPt[0], minPt[1], minPt[2]])
    point_d = np.array([minPt[0], maxPt[1], maxPt[2]])
    point_e = np.array([maxPt[0], minPt[1], maxPt[2]])
    point_f = np.array([maxPt[0], maxPt[1], minPt[2]])

    normals.append(getNormal(point_a, point_c, point_e))
    normals.append(getNormal(point_b, point_c, point_f))
    normals.append(getNormal(point_a, point_b, point_d))
    normals.append(getNormal(point_a, point_e, point_d))
    normals.append(getNormal(point_e, point_c, point_f))
    normals.append(getNormal(point_d, point_f, point_b))

    return normals

def main():

    tree = ET.parse(sys.argv[1])
    root = tree.getroot()

    # set default values
    viewDir=np.array([0,0,-1]).astype(np.float64)
    viewUp=np.array([0,1,0]).astype(np.float64)
    viewProjNormal=-1*viewDir  # you can safely assume this. (no examples will use shifted perspective camera)
    viewWidth=1.0
    viewHeight=1.0
    projDistance=1.0
    intensity=np.array([1,1,1]).astype(np.float64)  # how bright the light is.
    #print(np.cross(viewDir, viewUp))

    imgSize=np.array(root.findtext('image').split()).astype(np.int64)

    # list of the spheres and boxes
    objects = []
    light = []

    for c in root.findall('camera'):
        viewPoint = np.array(c.findtext('viewPoint').split()).astype(np.float64)
        viewDir = np.array(c.findtext('viewDir').split()).astype(np.float64)
        
        if (c.findtext('projNormal')):
            viewProjNormal = np.array(c.findtext('projNormal').split()).astype(np.float64)
        viewUp = np.array(c.findtext('viewUp').split()).astype(np.float64)
        
        if (c.findtext('projDistance')):
            projDistance = np.array(c.findtext('projDistance').split()).astype(np.float64)
        
        viewWidth = np.array(c.findtext('viewWidth').split()).astype(np.float64)
        viewHeight = np.array(c.findtext('viewHeight').split()).astype(np.float64)

    view = View(viewPoint, viewDir, viewUp, viewProjNormal, viewWidth, viewHeight, projDistance, intensity)

    for c in root.findall('surface'):
        type_c = c.get('type')
        
        if type_c == 'Sphere':
            center_c = np.array(c.findtext('center').split()).astype(np.float64)
            radius_c = np.array(c.findtext('radius')).astype(np.float64)
            ref = '' #ref의 data도 surface(object)의 내부에 저장
            for child in c:
                if child.tag == 'shader':
                    ref = child.get('ref')

            for d in root.findall('shader'):
                if d.get('name') == ref:
                    diffuse_d = np.array(d.findtext('diffuseColor').split()).astype(np.float64)
                    type_d = d.get('type')
                    
                    if type_d == 'Lambertian':
                        shader = ShaderLambertian(diffuse_d)
                        objects.append(Sphere(center_c, radius_c, shader))
                    
                    elif type_d == 'Phong':
                        exponent_d = np.array(d.findtext('exponent').split()).astype(np.float64)
                        specular_d = np.array(d.findtext('specularColor').split()).astype(np.float64)
                        shader = ShaderPhong(diffuse_d, specular_d, exponent_d)
                        objects.append(Sphere(center_c, radius_c, shader))
        
        elif type_c == 'Box':
            minPt = np.array(c.findtext('minPt').split()).astype(np.float64)
            maxPt = np.array(c.findtext('maxPt').split()).astype(np.float64)

            normals = makeCubeNormal(minPt, maxPt)

            point_a = np.array([minPt[0], minPt[1], maxPt[2]])
            point_b = np.array([minPt[0], maxPt[1], minPt[2]])
            point_c = np.array([maxPt[0], minPt[1], minPt[2]])
            point_d = np.array([minPt[0], maxPt[1], maxPt[2]])
            point_e = np.array([maxPt[0], minPt[1], maxPt[2]])
            point_f = np.array([maxPt[0], maxPt[1], minPt[2]])

            normals.append(getNormal(point_a, point_c, point_e))
            normals.append(getNormal(point_b, point_c, point_f))
            normals.append(getNormal(point_a, point_b, point_d))
            normals.append(getNormal(point_a, point_e, point_d))
            normals.append(getNormal(point_e, point_c, point_f))
            normals.append(getNormal(point_d, point_f, point_b))

            ref = '' #ref의 data도 surface(object)의 내부에 저장
            
            for child in c:
                if child.tag == 'shader':
                    ref = child.get('ref')
            
            for d in root.findall('shader'):
                if d.get('name') == ref:
                    diffuse_d = np.array(d.findtext('diffuseColor').split()).astype(np.float64)
                    type_d = d.get('type')
                    
                    if type_d == 'Lambertian':
                        shader = ShaderLambertian(diffuse_d)
                        objects.append(Box(minPt, maxPt, shader, normals))
                    
                    elif type_d == 'Phong':
                        exponent_d = np.array(d.findtext('exponent').split()).astype(np.float64)
                        specular_d = np.array(d.findtext('specularColor').split()).astype(np.float64)
                        shader = ShaderPhong(diffuse_d, specular_d, exponent_d)
                        objects.append(Box(minPt, maxPt, shader, normals))

    for c in root.findall('light'):
        position_c = np.array(c.findtext('position').split()).astype(np.float64)
        intensity_c = np.array(c.findtext('intensity').split()).astype(np.float64)
        light.append(Light(position_c, intensity_c))


    # Create an empty image (Height, Width, RGB channel)
    channels=3
    img = np.zeros((imgSize[1], imgSize[0], channels), dtype=np.uint8)
    img[:,:]=0
    
    # pixel's size
    pixel_x = view.viewWidth / imgSize[0]
    pixel_y = view.viewHeight / imgSize[1]

    w = view.viewDir
    u = np.cross(w, view.viewUp)
    v = np.cross(w, u)

    # make unit vector
    w_unit = w / np.sqrt(np.sum(w * w))
    u_unit = u / np.sqrt(np.sum(u * u))
    v_unit = v / np.sqrt(np.sum(v * v))

    # calculate start point
    start = w_unit * view.projDistance - u_unit * pixel_x * ((imgSize[0]/2) + 1/2) - v_unit * pixel_y * ((imgSize[1]/2) + 1/2)

    # replace the code block below! 
    for x in np.arange(imgSize[0]):
        for y in np.arange(imgSize[1]):
            ray = start + u_unit * x * pixel_x + pixel_y * y * v_unit
            temp = raytrace(objects, ray, view.viewPoint)
            img[y][x] = shade(temp[0], ray, view, objects, temp[1], light)

    rawimg = Image.fromarray(img, 'RGB')
    rawimg.save(sys.argv[1]+'.png')
    
if __name__=="__main__":
    main()
