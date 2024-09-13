from random import *
from PIL import Image,ImageFilter
from math import sqrt
map=[]
vector_map=[]

def normalize(v):
    d=v[0]*v[0]+v[1]*v[1]
    v[0]=v[0]/d
    v[1]=v[1]/d

for i in range(16):
    vector_map.append([])
    for j  in range(16):
        #vector_map[-1].append([-1,-1])
        vector_map[-1].append([((random()-0.5)*2),((random()-0.5)*2)])
        normalize(vector_map[-1][-1])

for i in range(64):
    map.append([])
    for j  in range(64):
        map[-1].append(0)  
img = Image.new(mode='RGB',size=(64,64))
dots={}
for x in range(64):
    for y  in range(64):
        p=[x-x%16+8,y-y%16+8]
        p1=[x-p[0]+0.5,y+0.5-p[1]]
        d=vector_map[y//16][x//16]
        dot=(d[0]*p1[0])+(d[1]*p1[1])
        #dot= sqrt(p1[0]**2+p1[1]**2)
        if((y//16,x//16) in  dots.keys()):
            dots[(y//16,x//16)].append(dot)
        else:
            dots[(y//16,x//16)]=[]
        map[y][x]=dot


for x in range(64):
    for y  in range(64):
        maxi=max(dots[(y//16,x//16)])
        d=map[y][x]
        d=d/maxi
        d=(d+1)/2
        d=d**6
        map[y][x]=int(d*250)


for x in range(64):
    for y  in range(64):
        img.putpixel((x,y),(map[y][x],map[y][x],map[y][x]))
#img = img.filter(ImageFilter.GaussianBlur(radius=5))
img.save('map.png')