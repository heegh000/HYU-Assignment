#!/usr/bin/env python3
# -*- coding: utf-8 -*
# see examples below
# also read all the comments below.

import os
import sys
import pdb  # use pdb.set_trace() for debugging
import code # or use code.interact(local=dict(globals(), **locals()))  for debugging.
import xml.etree.ElementTree as ET
import numpy as np
from PIL import Image

def normalize (vec) :
    return vec / np.sqrt(np.sum(vec * vec))

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

class Camera:
    def __init__(self, pos, view_vec, up_vec, dis, h, w, img_size_y, img_size_x):
        self.pos = pos
        self.u = normalize(np.cross(view_vec, up_vec))
        self.v = -normalize(np.cross(self.u, view_vec))
        self.w = normalize(np.cross(self.u, -self.v))
        self.dis = dis
        self.pix_y = h / img_size_y
        self.pix_x = w / img_size_x
        self.ip_origin = self.pos - (self.w * self.dis) 
        self.ray_start_pos = self.ip_origin - (self.v * self.pix_y * (img_size_y/2 + 0.5)) - (self.u * self.pix_x * (img_size_x/2+0.5)) 

class Ray:
    def __init__ (self, pos, dir_vec) :
        self.pos = pos
        self.dir_vec = dir_vec
    def set_t (self, min_t, min_idx) :
        self.min_t = min_t
        self.min_idx = min_idx

class Phong :
    shade = 'Phong'
    def __init__(self, name, diff, spec, expo) :
        self.name = name
        self.diff = diff
        self.spec = spec
        self.expo = expo

class Lambertian :
    shade = 'Lambertian'
    def __init__(self, name, diff) :
        self.name = name
        self.diff = diff


class Sphere:
    shape = 'Sphere'
    def __init__(self, radius, pos, shade_name) :
        self.radius = radius
        self.pos = pos
        self.shade_name = shade_name

class Box:
    shape = 'Box'
    def __init__(self, min_point, max_point, shade_name) :
        self.min_p = min_point
        self.max_p = max_point
        self.shade_name = shade_name


class Light : 
    def __init__(self, pos, inten) :
        self.pos = pos
        self.inten = inten
        

def raytrace (ray, shape_list) :

    min_t = sys.maxsize
    
    min_idx = -1
    curr_idx = -1

    # find the closest point
    for shape in shape_list :

        curr_idx += 1

        if shape.shape == 'Sphere':
            p = ray.pos - shape.pos
            d = ray.dir_vec

            dd = np.sum(d*d)
            pp = np.sum(p*p)
            dp = np.sum(d*p)
            
            if dp**2 - dd*(pp - shape.radius**2) >= 0:

                delta_t = np.sqrt(dp**2 - dd*(pp - shape.radius**2))
    
                if min_t >= (-dp + delta_t)/ dd:
                    min_t = (-dp + delta_t)/ dd
                    min_idx = curr_idx 

                if min_t >= (-dp - delta_t)/ dd:
                    min_t = (-dp - delta_t)/ dd
                    min_idx = curr_idx 
        else :

            min_t_box = (shape.min_p[0] - ray.pos[0]) / ray.dir_vec[0]
            max_t_box = (shape.max_p[0] - ray.pos[0]) / ray.dir_vec[0]
            
            if min_t_box > max_t_box :
                temp = min_t_box
                min_t_box = max_t_box
                max_t_box = temp

            min_y = (shape.min_p[1] - ray.pos[1]) / ray.dir_vec[1]
            max_y = (shape.max_p[1] - ray.pos[1]) / ray.dir_vec[1]

            if min_y > max_y :
                temp = min_y
                min_y = max_y
                max_y = temp


            if (min_t_box > max_y) or (min_y > max_t_box) :
                continue

            if min_y > min_t_box :
                min_t_box = min_y


            if max_y < max_t_box :
                max_t_box = max_y

            min_z = (shape.min_p[2] - ray.pos[2]) / ray.dir_vec[2]
            max_z = (shape.max_p[2] - ray.pos[2]) / ray.dir_vec[2]

            if min_z > max_z :
                temp = min_z
                min_z = max_z
                max_z = temp

            if (min_t_box > max_z) or (min_z > max_t_box) :
                continue 

            if min_z > min_t_box:
                min_t_box = min_z

            if max_z < max_t_box:
                max_t_box = max_z

            if min_t >= min_t_box :
                min_t = min_t_box
                min_idx = curr_idx
            

    ray.set_t (min_t, min_idx)

    if min_idx != -1 :
        return True
    else:
        return False

def shade (ray, light_list, shape_list, idx, shade_list) :

    color_r = 0
    color_g = 0
    color_b = 0

    target_shape = shape_list[idx]

    if target_shape.shape == 'Sphere':
        inter_vec = ray.pos + (ray.min_t * ray.dir_vec)

        normal_vec = normalize (inter_vec - target_shape. pos)

        for shade in shade_list: 
            if target_shape.shade_name == shade.name :
                target_shade = shade


        for light in light_list: 
            light_vec = normalize(light.pos - inter_vec)

            shadow_ray = Ray(light.pos, -light_vec)

            no_blocked = raytrace(shadow_ray, shape_list)

            if no_blocked and idx == shadow_ray.min_idx :
                if target_shade.shade == 'Phong' :
                    view_vec = normalize(-ray.dir_vec)
                    h_vec = normalize( view_vec + light_vec)
                    color_r = color_r + light.inten[0] * target_shade.diff[0] * max(0, np.sum(normal_vec * light_vec)) + light.inten[0] * target_shade.spec[0] * pow(max(0, np.sum(normal_vec * h_vec)), target_shade.expo[0])  
                    color_g = color_g + light.inten[1] * target_shade.diff[1] * max(0, np.sum(normal_vec * light_vec)) + light.inten[1] * target_shade.spec[1] * pow(max(0, np.sum(normal_vec * h_vec)), target_shade.expo[0])
                    color_b = color_b + light.inten[2] * target_shade.diff[2] * max(0, np.sum(normal_vec * light_vec)) + light.inten[2] * target_shade.spec[2] * pow(max(0, np.sum(normal_vec * h_vec)), target_shade.expo[0])  
                else: 
                    color_r = color_r + light.inten[0] * target_shade.diff[0] * max(0, np.sum(normal_vec * light_vec))
                    color_g = color_g + light.inten[1] * target_shade.diff[1] * max(0, np.sum(normal_vec * light_vec))
                    color_b = color_b + light.inten[2] * target_shade.diff[2] * max(0, np.sum(normal_vec * light_vec))
    
    else :
        inter_vec = ray.pos + (ray.min_t * ray.dir_vec)

        min_val =  abs(target_shape.min_p[0] - inter_vec[0])
        normal_vec = np.array([-1, 0, 0]).astype(np.float64)

        if  abs(target_shape.max_p[0]  - inter_vec[0]) < min_val :
            min_val = abs(target_shape.max_p[0]  - inter_vec[0] ) 
            normal_vec = np.array([1, 0, 0]).astype(np.float64)

        if  abs(target_shape.min_p[1]  - inter_vec[1]) < min_val :
            min_val = abs(target_shape.min_p[1]  - inter_vec[1]) 
            normal_vec = np.array([0, -1, 0]).astype(np.float64)

        if  abs(target_shape.max_p[1]  - inter_vec[1]) < min_val :
            min_val = abs(target_shape.max_p[1]  - inter_vec[1] )
            normal_vec = np.array([0, 1, 0]).astype(np.float64)

        if  abs(target_shape.min_p[2]  - inter_vec[2]) < min_val :
            min_val = abs(target_shape.min_p[2]  - inter_vec[2]) 
            normal_vec = np.array([0, 0, -1]).astype(np.float64)

        if  abs(target_shape.max_p[2]  - inter_vec[2]) < min_val :
            min_val = abs(target_shape.max_p[2]  - inter_vec[2] )
            normal_vec = np.array([0, 0, 1]).astype(np.float64)

        
        for shade in shade_list: 
            if target_shape.shade_name == shade.name :
                target_shade = shade

        for light in light_list: 
            light_vec = normalize(light.pos - inter_vec)

            shadow_ray = Ray(light.pos, -light_vec)

            no_blocked = raytrace(shadow_ray, shape_list)

            if no_blocked and idx == shadow_ray.min_idx :
                if target_shade.shade == 'Phong' :
                    view_vec = normalize(-ray.dir_vec)
                    h_vec = normalize( view_vec + light_vec)
                    color_r = color_r + light.inten[0] * target_shade.diff[0] * max(0, np.sum(normal_vec * light_vec)) + light.inten[0] * target_shade.spec[0] * pow(max(0, np.sum(normal_vec * h_vec)), target_shade.expo[0])  
                    color_g = color_g + light.inten[1] * target_shade.diff[1] * max(0, np.sum(normal_vec * light_vec)) + light.inten[1] * target_shade.spec[1] * pow(max(0, np.sum(normal_vec * h_vec)), target_shade.expo[0])
                    color_b = color_b + light.inten[2] * target_shade.diff[2] * max(0, np.sum(normal_vec * light_vec)) + light.inten[2] * target_shade.spec[2] * pow(max(0, np.sum(normal_vec * h_vec)), target_shade.expo[0])  
                else: 
                    color_r = color_r + light.inten[0] * target_shade.diff[0] * max(0, np.sum(normal_vec * light_vec))
                    color_g = color_g + light.inten[1] * target_shade.diff[1] * max(0, np.sum(normal_vec * light_vec))
                    color_b = color_b + light.inten[2] * target_shade.diff[2] * max(0, np.sum(normal_vec * light_vec))

    color = Color(color_r, color_g, color_b)
    color.gammaCorrect(2.2)
    return color    

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

    imgSize=np.array(root.findtext('image').split()).astype(np.int64)


    shade_list = []

    shape_list = []

    light_list = []
    
    for c in root.findall('camera'):
        viewPoint=np.array(c.findtext('viewPoint').split()).astype(np.float64)
        viewDir = np.array(c.findtext('viewDir').split()).astype(np.float64)
        viewProjNormal = np.array(c.findtext('projNormal').split()).astype(np.float64)
        viewUp = np.array(c.findtext('viewUp').split()).astype(np.float64)
        if (c.findtext('projDistance')):
            projDistance = np.array(c.findtext('projDistance').split()).astype(np.float64)
        viewWidth = np.array(c.findtext('viewWidth').split()).astype(np.float64)
        viewHeight = np.array(c.findtext('viewHeight').split()).astype(np.float64)



    for c in root.findall('shader'):
        shade_type = c.get('type')
        shade_name = c.get('name')
        if shade_type == 'Phong' :
            diff = np.array(c.findtext('diffuseColor').split()).astype(np.float64)
            spec = np.array(c.findtext('specularColor').split()).astype(np.float64)
            expo = np.array(c.findtext('exponent').split()).astype(np.float64)
            shade_list.append(Phong(shade_name, diff, spec, expo))
        else :
            diff =np.array(c.findtext('diffuseColor').split()).astype(np.float64)
            shade_list.append(Lambertian(shade_name, diff))

    for c in root.findall('surface'):
        surface_type = c.get('type')
        if surface_type == 'Sphere' :
            shade_name = ''
            for cc in c :
                if cc.tag == 'shader' :
                    shade_name = cc.get('ref')
            pos = np.array(c.findtext('center').split()).astype(np.float64)
            raidus =  np.array(c.findtext('radius').split()).astype(np.float64) 
            shape_list.append(Sphere(raidus, pos, shade_name))
        else :
            shade_name = ''
            for cc in c :
                if cc.tag == 'shader' :
                    shade_name = cc.get('ref')
            minPt = np.array(c.findtext('minPt').split()).astype(np.float64)
            maxPt = np.array(c.findtext('maxPt').split()).astype(np.float64)
            shape_list.append(Box(minPt, maxPt, shade_name))


    for c in root.findall('light'):
        light_pos = np.array(c.findtext('position').split()).astype(np.float64)
        intensity = np.array(c.findtext('intensity').split()).astype(np.float64)
        light_list.append(Light(light_pos, intensity))


    #code.interact(local=dict(globals(), **locals()))  

    # Create an empty image
    channels=3
    img = np.zeros((imgSize[1], imgSize[0], channels), dtype=np.uint8)
    img[:,:]=0

    camera = Camera(viewPoint, viewDir, viewUp, projDistance, viewHeight, viewWidth, imgSize[1], imgSize[0])

    for y in np.arange(imgSize[1]) :
        for x in np.arange(imgSize[0]) : 
            dir_vec = camera.ray_start_pos + (camera.pix_y * y * camera.v) + (camera.pix_x * x * camera.u) - camera.pos
            ray = Ray(camera.pos, dir_vec)
            not_blocked = raytrace(ray, shape_list)
            

            if not_blocked :
                color = shade(ray, light_list, shape_list, ray.min_idx ,shade_list)
            else: 
                color = Color(0, 0, 0)

            img[y][x] = color.toUINT8()
            


    '''
    # replace the code block below!o
    for i in np.arange(imgSize[1]): 
        white=Color(1,1,1)
        red=Color(1,0,0)
        blue=Color(0,0,1)
        img[10][i]=white.toUINT8()
        img[i][i]=red.toUINT8()
        img[i][0]=blue.toUINT8()

    for x in np.arange(imgSize[0]): 
        img[5][x]=[255,255,255]
    '''

    rawimg = Image.fromarray(img, 'RGB')
    #rawimg.save('out.png')
    rawimg.save(sys.argv[1]+'.png')
    
if __name__=="__main__":
    main()
