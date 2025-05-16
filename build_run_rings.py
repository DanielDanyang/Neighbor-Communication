#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import subprocess
import sys
import time

def print_separator():
    print("\n" + "="*50 + "\n")

def compile_program(source_file, output_file):
    """编译MPI程序"""
    print(f"编译 {source_file} 为 {output_file}")
    result = subprocess.run(["mpicxx", "-o", output_file, source_file], 
                            capture_output=True, text=True)
    
    if result.returncode != 0:
        print(f"编译错误: {result.stderr}")
        return False
    
    print(f"编译成功: {output_file}")
    return True

def run_program(program_name, num_processes=8):
    """运行MPI程序"""
    print(f"使用 {num_processes} 个进程运行 {program_name}")
    print_separator()
    
    result = subprocess.run(["mpirun", "-np", str(num_processes), f"./{program_name}"],
                           capture_output=False)
    
    if result.returncode != 0:
        print(f"运行错误，返回代码: {result.returncode}")
    
    print_separator()

def main():
    # 确保工作目录
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    
    # 定义源文件和可执行文件
    programs = [
        {"source": "ring.cpp", "output": "ring"},
        {"source": "ring1.cpp", "output": "ring1"},
        {"source": "ring2.cpp", "output": "ring2"},
        {"source": "ring3.cpp", "output": "ring3"}
    ]
    
    # 获取进程数量参数
    num_processes = 8  # 默认值
    if len(sys.argv) > 1:
        try:
            num_processes = int(sys.argv[1])
        except ValueError:
            print(f"无效的进程数: {sys.argv[1]}，将使用默认值8")
    
    # 编译所有程序
    all_compiled = True
    for program in programs:
        if not os.path.exists(program["source"]):
            print(f"警告: 源文件 {program['source']} 不存在，跳过")
            continue
            
        if not compile_program(program["source"], program["output"]):
            all_compiled = False
    
    if not all_compiled:
        print("有些程序编译失败，请检查错误信息")
        return
    
    # 运行所有程序
    print("\n开始运行所有程序...\n")
    for program in programs:
        if os.path.exists(program["output"]):
            print(f"\n运行程序: {program['output']}")
            run_program(program["output"], num_processes)
            # 休息一秒，避免输出混淆
            time.sleep(1)
    
    print("所有程序执行完毕！")

if __name__ == "__main__":
    main() 