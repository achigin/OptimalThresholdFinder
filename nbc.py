#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division
from collections import defaultdict
from math import log, exp
import random
import os

def train(samples):
    classes, freq = defaultdict(lambda:0), defaultdict(lambda:0)
    for feats, label in samples:
        classes[label] += 1                 # count classes frequencies
        for feat in feats:
            freq[label, feat] += 1          # count features frequencies

    for label, feat in freq:                # normalize features frequencies
        freq[label, feat] /= classes[label]
    for c in classes:                       # normalize classes frequencies
        classes[c] /= len(samples)

    return classes, freq                    # return P(C) and P(O|C)

def classify2(classifier, feats):
    classes, prob = classifier
    class_res = dict()
    for i, item in enumerate(classes.keys()):
        value = -log(classes[item]) + sum(-log(prob.get((item, feat), 10**(-7))) for feat in feats)
        if (item is not None):
            class_res[item] = value
    eps = 709.0
    posVal = '1'
    negVal = '0'
    posProb = negProb = 0
    if (abs(class_res[posVal] - class_res[negVal]) < eps):
        posProb = 1.0 / (1.0 + exp(class_res[posVal] - class_res[negVal]))
        negProb = 1.0 / (1.0 + exp(class_res[negVal] - class_res[posVal]))
    else:
        if class_res[posVal] > class_res[negVal]:
            posProb = 0.0
            negProb = 1.0
        else:
            posProb = 1.0
            negProb = 0.0
    return str(posProb) + '\t' + str(negProb)



def get_features_complex(sample): return (
        'll: %s' % sample[-1],          # get last letter
        'fl: %s' % sample[0],           # get first letter
        'sl: %s' % sample[1],           # get second letter
)

def get_features_simple(sample): return (sample[-1],)

def main(get_features, train_file, input_file, output_file):
    samples = (line.decode('utf-8').split() for line in open(train_file))
    features = [(get_features(feat), label) for feat, label in samples]
    classifier = train(features)
    test_samples = (line.decode('utf-8').split() for line in open(input_file))
    with open(output_file, 'w') as f1:
        for feat, label in test_samples:
            data = feat + '\t' + classify2(classifier, get_features(feat)) + '\t' + label + '\n'
            data = data.encode('utf-8')
            f1.write(data)

def cross_validate(get_features, fold_count, input_file, output_file):
    if fold_count >= 2:
        folds = dict()
        for i in range(0, fold_count):
            folds[i] = open(input_file + '_fold_' + str(i), 'w')
        with open(input_file) as f:
            for line in f:
                fold_prob = random.randint(0, fold_count - 1)
                folds[fold_prob].write(line)
        for i in range(0, fold_count):
            folds[i].close()

        for i in range(0, fold_count):
            fold_train = open(input_file + '_fold_' + str(i) + '_train', 'w')
            fold_validate = open(input_file + '_fold_' + str(i) + '_validate', 'w')
            for j in range(0, fold_count):
                if i != j:
                    fold_write = fold_train
                else:
                    fold_write = fold_validate
                with open(input_file + '_fold_' + str(j)) as f:
                    for line in f:
                        fold_write.write(line)
            fold_validate.close()
            fold_train.close()

        for i in range(0, fold_count):
            os.remove(input_file + '_fold_' + str(i))
            main(get_features,
                 input_file + '_fold_' + str(i) + '_train',
                 input_file + '_fold_' + str(i) + '_validate',
                 output_file + '_fold_res_' + str(i))
            os.remove(input_file + '_fold_' + str(i) + '_train')
            os.remove(input_file + '_fold_' + str(i) + '_validate')


cross_validate(get_features_simple, 10, 'names_full_pool.txt', 'names_full_pool_simple.txt')
cross_validate(get_features_complex, 10, 'names_full_pool.txt', 'names_full_pool_complex.txt')
main(get_features_simple, 'names_learn_pool.txt', 'names_test_pool.txt', 'names_test_pool.txt_simple')
main(get_features_complex, 'names_learn_pool.txt', 'names_test_pool.txt', 'names_test_pool.txt_complex')
