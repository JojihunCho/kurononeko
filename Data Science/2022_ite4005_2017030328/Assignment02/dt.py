import argparse
import random
import numpy as np

from collections import defaultdict
from multiprocessing import Value


# argument parser
parser = argparse.ArgumentParser()
parser.add_argument("training", help="training file name", type=str)
parser.add_argument("test", help="test file name", type=str)
parser.add_argument("output", help="output file name", type=str)
args = parser.parse_args()

#구한 규칙을 이용해 분석을 하는 함수 1개의 line 씩 진행
def testing(rule, data, infoma, attribute):
    vlaue = []
    i = rule[0]
    j = rule
    while(i in infoma):
        temp = i
        view = attribute[infoma.index(temp)]
        next = view.index(data[infoma.index(temp)])
        temp = j[next + 1][0]
        if(temp in infoma):
            i = temp
        else:
            i = random.choice(j[next + 1])
        if(next + 1 < len(j)):
            j = j[next + 1]
        
    vlaue = i
    return vlaue

#트레이닝 함수
def makeRule(data, infoma, attribute, target):
    rule = []
    tree, criteria = infoGain(data, infoma, attribute, target)
    rule.append(criteria)
    if(criteria not in infoma):
        for i in tree:
            rule.append(i[-1:])
        rule.remove('')
        return rule
    '''for i in range(len(attribute[infoma.index(criteria)])):
        rule.append([])'''
    
    for i in range(len(tree)):
        rule.append(makeRule(tree[i], infoma, attribute, target))
    #print(rule)
    return rule

#information gain을 기준으로 
#best tree: 분류된 이후의 최고의 tree를 의미. best criteria: 최고의 분류 기준. 없으면 null
def infoGain(subdata, infoma, attribute, target):
    best_tree = subdata
    best_gain = 0. #초기 값 설정 (pro_entro = now_entropy)
    best_criteria = ""

    pro_entro = Entropy(subdata, target) #기존 엔트로피 계산
    
    #분류할 만들 트리 생성
    for mark in range(len(attribute)):
        new_entro = 0.
        new_tree = []
        for i in range(len(attribute[mark])):
            new_tree.append([])

        for data in subdata:
            new_tree[attribute[mark].index(data[mark])].append(data) #새로운 트리 분류
        
        for node in new_tree:
            new_entro += len(node)/len(subdata) * Entropy(node, target)
        info_gain = pro_entro - new_entro

        if (info_gain > best_gain):
            best_gain = info_gain
            best_tree = new_tree
            best_criteria = infoma[mark]

    return best_tree, best_criteria

#p는 결과의 개수를 비율로 표현한 list
def Entropy(tree, target):
    p = np.zeros(len(target))
    for data in tree:
        p[target.index(data[len(data) - 1])] += 1

    for i in range(len(p)):
        p[i] = p[i]/len(tree)
    
    id_p = np.where(p != 0)
    return -np.sum(p[id_p]*np.log(p[id_p]))

#각 속성이 보유하는 값 추출
def initdata(data, num):
    target = []
    for item in data:
        temp = item[num]
        if temp not in target:
            target.append(temp)
    return target

#attribute = 속성이 가질 수 있는 값, target = 결과가 가질 수 있는 값
if __name__ == '__main__':
    attribute = []
    with open(args.training) as f:
        data = [line.split() for line in f.readlines()]
    infoData = data[0]
    
    #
    i = 0
    while(i < len(data[0]) - 1) :
        attribute.append(initdata(data[1:], i))
        i = i + 1
    target = initdata(data[1:], i)
    
    rule = makeRule(data[1:], infoData, attribute, target)
    #print(rule)
    #print(Entropy(data[1:], target))
    #tree, criteria = infoGain(data[1:], infoData, attribute, target)
    '''for i in rule:
        print(i)'''

    with open(args.test) as f:
        data = [line.split() for line in f.readlines()]
    
    result = []
    result.append(infoData)
    for i in data[1:]:
        input = testing(rule, i, infoData, attribute)
        i.append(input[0])
        result.append(i)
        #print(i)

    with open(args.output, 'w') as f:
        for output in result:
            f.write('\t'.join(output))
            f.write('\n')
