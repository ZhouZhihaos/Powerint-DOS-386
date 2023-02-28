#!/usr/bin/python3 
#
# SPDX-License-Identifier: LGPL-2.1-only
# -------------------------------*-TayhuangOS-*-----------------------------------
# 
#    Copyright (C) 2022, 2022 TayhuangOS Development Team - All Rights Reserved
# 
# --------------------------------------------------------------------------------
# 
# 作者: theflysong
# 
# comments_stat.py
# 
# 注释统计工具
# 
#



import os

look_dirs = ['./']
    
level = 0

def calc_density(code, comment):
    if code + comment == 0:
        return 1
    return comment / (code + comment)

def stat_comment(fp):
    global level
    file_names = fp.rsplit('.', maxsplit = 1)

    # 没有后缀名
    if len(file_names) != 2:
        return 0, 0

    # 不是c文件
    if (file_names[1] != 'c') & (file_names[1] != 'h'):
        return 0, 0

    comment_num = 0
    code_num = 0
    flag = 0
    with open(fp, 'r',encoding="UTF-8") as f:
        for line in f:
            # 去掉空格
            line = line.strip()

            # 空行
            if not line:
                continue

            if flag != 0:
                if flag == 1:
                    if line.startswith('* SPDX-License-Identifier: LGPL-2.1-only'): #忽略许可证
                        flag = 3
                        continue
                    comment_num = comment_num + 1
                    flag = 2

                if flag == 2:
                    comment_num = comment_num + 1

                if line.find('*/') != -1:
                    flag = 0

                continue

            # 是//开头的注释
            if line.startswith('//'):
                comment_num = comment_num + 1
                continue

            # 含有/*
            if line.find('/*') != -1:
                if line.find('*/') == -1:
                    flag = 1
                else:
                    comment_num = comment_num + 1
                continue

            #是代码
            code_num = code_num + 1

            if line.find('//') != -1:
                comment_num = comment_num + 1

    for i in range(level):
        print ('  ', end='')

    print ("(" + fp + "): code=" + str(code_num) + " comment=" + str(comment_num) + " comment density=" + (str(calc_density(code_num, comment_num) * 100)) + "%")
    return code_num, comment_num

def stat(look_dir):
    global level
    comment_num = 0
    code_num = 0
    for i in range(level):
        print (' ', end='')
    print ("{")
    for root, dirs, files in os.walk(look_dir):
        for f in files:
            level = level + 1
            fcode, fcomment = stat_comment(os.path.join(root, f))
            level = level - 1
            code_num += fcode
            comment_num += fcomment
    for i in range(level):
        print (' ', end='')
    print ("}")
    for i in range(level):
        print (' ', end='')
    print ("(" + look_dir + "): code=" + str(code_num) + " comment=" + str(comment_num) + " comment density=" + (str(calc_density(code_num, comment_num) * 100)) + "%")
    return code_num, comment_num



comment_num = 0
code_num = 0
level = level + 1
for look_dir in look_dirs:
    print ("{")
    dcode, dcomment = stat(look_dir)
    code_num += dcode
    comment_num += dcomment
    print ("}")
print ("(./):" + " code=" + str(code_num) + " comment=" + str(comment_num) + " comment density=" + (str(calc_density(code_num, comment_num) * 100)) + "%")