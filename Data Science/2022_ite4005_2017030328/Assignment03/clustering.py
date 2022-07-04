import argparse
from re import L
import numpy as np


# argument parser
parser = argparse.ArgumentParser()
parser.add_argument("input", help="input file name", type=str)
parser.add_argument("clust_num", help="number of clusters for the corresponding input data", type=int)
parser.add_argument("esp", help="maximum radius of the neighborhood", type=float)
parser.add_argument("minpts", help="minimum number of points in an Eps-neighborhood of a given point", type=int)
args = parser.parse_args()

now_cluster = -1

def distence(pointA, pointB):
    dist = 0
    dist = np.sqrt(np.sum(np.square(pointA-pointB)))
    return dist

def clustering(point_id, point, point_list, point_type, point_processed):
    global now_cluster
    neighbor = []
    #print(point_id + " - start function")
    pid = int(point_id) #first point of cluster
    for i in point_list:
        dist = distence(point, np.array([i[1], i[2]]))
        if args.esp >= dist:
            neighbor.append(i)
        
    if len(neighbor) >= (args.minpts): #check core
        now_cluster += 1
        #print(point_id + ' is in ' + str(now_cluster))
        point_type[pid] = now_cluster
        point_processed[pid] = True
    else: #if point is not core, end function
        return 0

    for i in neighbor:
        point_type[int(i[0])] = now_cluster #set neighbor cluster

    for i in neighbor: #secend and after than point of cluster
        near_point = []
        if point_processed[int(i[0])] != True:
            point_processed[int(i[0])] = True
            for j in point_list:
                dist = distence(np.array([i[1], i[2]]), np.array([j[1], j[2]]))

                if args.esp >= dist:
                    near_point.append(j)

            if len(near_point) >= (args.minpts):
                #print(i[0])
                for temp in near_point:
                    if temp not in neighbor:
                        point_type[int(temp[0])] = now_cluster
                        neighbor.append(temp)
            #clustering(i[0], np.array([i[1], i[2]]), point_list, point_type, point_processed)
    
    return len(neighbor)

    

if __name__ == '__main__':
    point_type = []
    processed = []
    able_cluster = []
    min_cluster = 0

    with open(args.input) as f:
        data = [line.split() for line in f.readlines()]

    for point in data:
        point[1] = float(point[1])
        point[2] = float(point[2])
        point_type.append(-1) # -1 = 'outlier' , 0 <= 'cluster'
        processed.append(False)

    for point in data:
        point_data = np.array([point[1], point[2]])
        if processed[int(point[0])] != True:
            mem_num = clustering(point[0], point_data, data, point_type, processed)
            if mem_num > 0:
                able_cluster.append([now_cluster, mem_num])

            
    #print(able_cluster)
    write_cluster = []
    if len(able_cluster) > args.clust_num:
        while len(write_cluster) < args.clust_num:
            max = 0
            temp = [-1 , 0]
            for i in able_cluster:
                if i[1] > max:
                    max = i[1]
                    temp = i
            write_cluster.append(temp)
            able_cluster.remove(temp)
    else:
        write_cluster = able_cluster

    fileable = []
    for i in write_cluster:
        fileable.append(i[0])

    #print(fileable)

    output_file = []

    for i in range(0, args.clust_num):
        txt = args.input.split('.')[0] + '_cluster_'+ str(i) +'.txt'
        file = open(txt, 'w')
        output_file.append(file)

    for point_id in range(len(data)):
        point = data[point_id]
        if point_type[point_id] in fileable:
            output_file[fileable.index(point_type[point_id])].write(str(point_id) + '\n')

    for f in output_file:
        f.close()