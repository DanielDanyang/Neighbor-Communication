#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess
import sys
import time
import csv
import re

def print_separator():
    print("\n" + "="*50 + "\n")

def remove_old_files(output_file):
    """Remove previous executable and job files"""
    # Remove old executable if exists
    if os.path.exists(output_file):
        try:
            os.remove(output_file)
            print("Removed previous executable: {0}".format(output_file))
        except OSError as e:
            print("Error removing file {0}: {1}".format(output_file, e))
    
    # Remove old job script if exists
    job_script = "{0}_job.sh".format(output_file)
    if os.path.exists(job_script):
        try:
            os.remove(job_script)
            print("Removed previous job script: {0}".format(job_script))
        except OSError as e:
            print("Error removing file {0}: {1}".format(job_script, e))

def create_job_script(program_name, num_processes, message_size, job_id):
    """Create a Slurm job script with message size parameter"""
    output_name = "{0}_procs{1}_size{2}_{3}".format(program_name, num_processes, message_size, job_id)
    script_content = """#!/bin/bash
#SBATCH --job-name={0}_p{1}
#SBATCH --nodes=1
#SBATCH --ntasks={1}
#SBATCH --mem=16G
#SBATCH --time=00:10:00
#SBATCH --output={4}.out
#SBATCH --error={4}.err

module load mpi

mpirun -np {1} ./{0} {2}
""".format(program_name, num_processes, message_size, job_id, output_name)
    
    script_name = "{0}_procs{1}_size{2}_{3}_job.sh".format(program_name, num_processes, message_size, job_id)
    with open(script_name, "w") as f:
        f.write(script_content)
    return script_name, output_name

def compile_program(source_file, output_file):
    """Compile MPI program"""
    print("Compiling {0} to {1}".format(source_file, output_file))
    
    # Remove old executable first
    remove_old_files(output_file)
    
    process = subprocess.Popen(["mpicxx", "-o", output_file, source_file, "-lm"], 
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    _, stderr = process.communicate()
    
    if process.returncode != 0:
        print("Compilation error: {0}".format(stderr))
        return False
    
    print("Compilation successful: {0}".format(output_file))
    return True

def submit_job(program_name, num_processes, message_size, job_id):
    """Submit job using sbatch with message size parameter"""
    print("Submitting job for {0} with {1} processes, message size: {2}".format(
        program_name, num_processes, message_size))
    
    # Create job script
    job_script, output_name = create_job_script(program_name, num_processes, message_size, job_id)
    
    # Submit job
    process = subprocess.Popen(["sbatch", job_script], 
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    
    if process.returncode != 0:
        print("Job submission error: {0}".format(stderr))
        return False, None
    
    print("Job submitted successfully: {0}".format(stdout.strip()))
    return True, output_name

def collect_results(output_files):
    """Collect and analyze results from all output files"""
    results = {}
    
    # Process available output files
    for output_file in output_files:
        if not os.path.exists(output_file + ".out"):
            print("Warning: Output file {0}.out not found, skipping".format(output_file))
            continue
        
        # Parse the output file name to get program, processes, size and job_id
        match = re.match(r'(\w+)_procs(\d+)_size(\d+)_(\w+)', output_file)
        if not match:
            print("Warning: Output file name {0} does not match expected format, skipping".format(output_file))
            continue
        
        program = match.group(1)
        num_processes = int(match.group(2))
        message_size = int(match.group(3))
        job_id = match.group(4)
        
        # Initialize result structures if needed
        if program not in results:
            results[program] = {}
        if num_processes not in results[program]:
            results[program][num_processes] = {}
        if message_size not in results[program][num_processes]:
            results[program][num_processes][message_size] = []
        
        # Parse the output file for communication times
        comm_times = []
        max_time = 0.0  # Track maximum communication time across all processes
        with open(output_file + ".out", 'r') as f:
            for line in f:
                if "communication time:" in line:
                    parts = line.strip().split("communication time:")
                    if len(parts) > 1:
                        time_parts = parts[1].split("seconds")
                        if len(time_parts) > 0:
                            try:
                                comm_time = float(time_parts[0].strip())
                                comm_times.append(comm_time)
                                max_time = max(max_time, comm_time)
                            except ValueError:
                                print("Warning: Could not parse communication time from line: {0}".format(line))
        
        # Store the average and maximum communication time
        if comm_times:
            avg_time = sum(comm_times) / len(comm_times)
            results[program][num_processes][message_size].append((avg_time, max_time))
            print("Program: {0}, Processes: {1}, Size: {2}, Job: {3}, Avg Time: {4:.6f} seconds, Max Time: {5:.6f} seconds".format(
                program, num_processes, message_size, job_id, avg_time, max_time))
        else:
            print("Warning: No communication times found in {0}.out".format(output_file))
    
    return results

def generate_csv_report(results, filename="performance_results.csv"):
    """Generate CSV report from results"""
    if not results:
        print("No results to report")
        return
    
    # Write CSV file for performance vs message size - Python 2.7 compatible
    with open(filename, 'wb') as f:
        writer = csv.writer(f)
        
        # Write header
        header = ["Program", "Processes", "Message Size", "Avg Time (s)", "Max Time (s)"]
        writer.writerow(header)
        
        # Write data rows
        for program in sorted(results.keys()):
            for num_processes in sorted(results[program].keys()):
                for message_size in sorted(results[program][num_processes].keys()):
                    runs = results[program][num_processes][message_size]
                    if runs:
                        # Calculate average across all runs
                        avg_times = [run[0] for run in runs]
                        max_times = [run[1] for run in runs]
                        avg_time = sum(avg_times) / len(avg_times)
                        max_time = sum(max_times) / len(max_times)
                        writer.writerow([program, num_processes, message_size, 
                                         "{:.6f}".format(avg_time), "{:.6f}".format(max_time)])
    
    print("\nPerformance results saved to {0}".format(filename))
    
    # Generate scalability report
    scalability_filename = "scalability_results.csv"
    with open(scalability_filename, 'wb') as f:
        writer = csv.writer(f)
        
        # Write header
        header = ["Program", "Message Size"]
        # Find all process counts across all programs
        all_processes = set()
        for program in results:
            all_processes.update(results[program].keys())
        all_processes = sorted(all_processes)
        
        # Add process counts to header
        for proc in all_processes:
            header.append(str(proc) + " procs")
        writer.writerow(header)
        
        # Write data rows grouped by program and message size
        for program in sorted(results.keys()):
            message_sizes = set()
            for proc_data in results[program].values():
                message_sizes.update(proc_data.keys())
            
            for message_size in sorted(message_sizes):
                row = [program, message_size]
                for proc in all_processes:
                    if proc in results[program] and message_size in results[program][proc]:
                        runs = results[program][proc][message_size]
                        if runs:
                            avg_times = [run[0] for run in runs]
                            avg_time = sum(avg_times) / len(avg_times)
                            row.append("{:.6f}".format(avg_time))
                        else:
                            row.append("N/A")
                    else:
                        row.append("N/A")
                writer.writerow(row)
    
    print("Scalability results saved to {0}".format(scalability_filename))
    
    # Generate efficiency report (speedup/process count)
    efficiency_filename = "efficiency_results.csv"
    with open(efficiency_filename, 'wb') as f:
        writer = csv.writer(f)
        
        # Write header
        header = ["Program", "Message Size"]
        for proc in all_processes:
            if proc > 1:  # Only include process counts > 1 for efficiency/speedup
                header.append(str(proc) + " procs")
        writer.writerow(header)
        
        # Write data rows grouped by program and message size
        for program in sorted(results.keys()):
            message_sizes = set()
            for proc_data in results[program].values():
                message_sizes.update(proc_data.keys())
            
            for message_size in sorted(message_sizes):
                row = [program, message_size]
                
                # Find the base case (lowest process count) time for speedup calculation
                base_procs = min(results[program].keys())
                if message_size in results[program][base_procs]:
                    base_runs = results[program][base_procs][message_size]
                    if base_runs:
                        base_avg_times = [run[0] for run in base_runs]
                        base_time = sum(base_avg_times) / len(base_avg_times)
                    else:
                        base_time = None
                else:
                    base_time = None
                
                # Calculate efficiency for each process count (speedup/process count)
                for proc in all_processes:
                    if proc <= 1 or base_time is None:  # Skip base case and invalid cases
                        continue
                        
                    if proc in results[program] and message_size in results[program][proc]:
                        runs = results[program][proc][message_size]
                        if runs:
                            avg_times = [run[0] for run in runs]
                            avg_time = sum(avg_times) / len(avg_times)
                            speedup = base_time / avg_time
                            efficiency = speedup / proc
                            row.append("{:.4f}".format(efficiency))
                        else:
                            row.append("N/A")
                    else:
                        row.append("N/A")
                
                writer.writerow(row)
    
    print("Efficiency results saved to {0}".format(efficiency_filename))

def main():
    # Ensure working directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    
    # Define source files and executables
    programs = [
        # {"source": "ring.cpp", "output": "ring"},
        {"source": "ring1.cpp", "output": "ring1"},
        {"source": "ring2.cpp", "output": "ring2"},
        {"source": "ring3.cpp", "output": "ring3"}
    ]
    
    # 测试的消息大小 - 使用一个大小简化测试
    message_sizes = [5000000]  # 5MB 大小，足以体现性能差异
    
    # 测试不同进程数量以分析可扩展性
    process_counts = [2, 4, 8, 16, 32]  # 根据集群可用资源调整
    
    # 检查命令行参数是否指定了特定程序
    selected_programs = []
    if len(sys.argv) > 1 and sys.argv[1] not in ["--collect"]:
        for arg in sys.argv[1:]:
            if arg.startswith("--"):
                continue  # 跳过选项参数
            for program in programs:
                if program["output"] == arg:
                    selected_programs.append(program)
                    break
        if not selected_programs:
            print("Warning: No valid program names provided. Using all programs.")
            selected_programs = programs
    else:
        selected_programs = programs
    
    # 每个配置的运行次数
    num_runs = 1
    
    # Compile selected programs
    all_compiled = True
    for program in selected_programs:
        if not os.path.exists(program["source"]):
            print("Warning: Source file {0} does not exist, skipping".format(program["source"]))
            continue
            
        if not compile_program(program["source"], program["output"]):
            all_compiled = False
    
    if not all_compiled:
        print("Some programs failed to compile, please check error messages")
        return
    
    # Submit jobs for all programs, process counts and message sizes
    output_files = []
    job_id = int(time.time())  # Use timestamp as job identifier
    
    print("\nSubmitting jobs for different process counts and message sizes...")
    for program in selected_programs:
        if not os.path.exists(program["output"]):
            continue
        
        for num_processes in process_counts:
            for message_size in message_sizes:
                for run in range(num_runs):
                    run_job_id = "{0}_run{1}".format(job_id, run)
                    print_separator()
                    success, output_name = submit_job(program["output"], num_processes, message_size, run_job_id)
                    if success and output_name:
                        output_files.append(output_name)
                    # Small delay between submissions
            time.sleep(1)
    
    print_separator()
    print("All jobs have been submitted!")
    print("\nUse 'squeue' to check job status")
    print("Output will be available in [program_name]_procs[count]_size[size]_[jobid].out files")
    
    # 直接保存结果，不再询问用户
    print("\nAll job names have been saved to pending_jobs.txt")
    print("Run the script with --collect when jobs are finished to collect results")
    
    # Save the output file names for later collection
    with open("pending_jobs.txt", "w") as f:
        for output_file in output_files:
            f.write(output_file + "\n")

if __name__ == "__main__":
    # Check if we're just collecting results from previous runs
    if len(sys.argv) > 1 and sys.argv[1] == "--collect":
        if os.path.exists("pending_jobs.txt"):
            output_files = []
            with open("pending_jobs.txt", "r") as f:
                for line in f:
                    output_files.append(line.strip())
            print("Found {0} pending jobs to collect results from".format(len(output_files)))
            results = collect_results(output_files)
            if results:
                generate_csv_report(results)
                print("\nScalability analysis complete!")
        else:
            print("No pending_jobs.txt file found. Run the script first to submit jobs.")
    else:
    main() 