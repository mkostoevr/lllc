#!/usr/bin/python3

import os
import sys
from importlib.machinery import SourceFileLoader
from multiprocessing.pool import ThreadPool
from threading import Lock
import traceback
import timeit

tests = []
failed_tests = []
finished_tests = [] # Both succeed and failed
callback_lock = Lock()
test_start_times = {}
root_dir = os.getcwd()
test_time = 0.0

def collect_tests(path):
    tests = []
    for test_folder in os.listdir(path):
        test_folder_path = f"{path}/{test_folder}"
        test_file = f"{test_folder_path}/test.py"

        if not os.path.isdir(test_folder_path):
            continue

        if os.path.exists(test_file):
            tests.append(test_folder_path)
        else:
            # Recurse into the folder, maybe there're test folders in it
            tests += collect_tests(test_folder_path)
    return tests

def get_progress():
    return f"[{len(finished_tests) + 1}/{len(tests)}]"

def get_status_from_exception(ex):
    if type(ex) == common.TestFailureException:
        status = "FAILURE"
    elif type(ex) == common.TestTimeoutException:
        status = "TIMEOUT"
    else:
        status = "UNKNOWN"

def get_test_file_from_exception(ex):
    tb = traceback.extract_tb(ex.__traceback__)
    for frame in tb:
        if frame.filename.endswith("test.py"):
            # The filename is full, let's extract just relative test folder
            return frame.filename.split(f"{root_dir}/")[1].split("/test.py")[0]

def run_test(root_dir, test):
    test_dir = f"{root_dir}/{test}"
    test_file = f"{test_dir}/test.py"

    test_start_times[test] = timeit.default_timer()
    SourceFileLoader("test", test_file).load_module().run(root_dir, test_dir)
    return test

def callback(arg):
    global test_time
    with callback_lock:
        test = arg
        time = timeit.default_timer() - test_start_times[test]
        test_time += time
        progress = get_progress()
        print(f"{progress} {test}: SUCCESS ({time:.2f} seconds)", flush = True)
        finished_tests.append(test)

def error_callback(ex):
    global test_time
    with callback_lock:
        status = get_status_from_exception(ex)
        test = get_test_file_from_exception(ex)
        time = timeit.default_timer() - test_start_times[test]
        test_time += time
        progress = get_progress()
        print(f"{progress} {test}: {status} ({time:.2f} seconds)", flush = True)
        finished_tests.append(test)
        failed_tests.append(test)

if __name__ == '__main__':
    sys.path.append('test')
    import common

    tests = collect_tests('test')

    # Execute each test
    test_executors = ThreadPool()
    test_start = timeit.default_timer()
    for test in tests:
        test_executors.apply_async(run_test,
                                   args = (root_dir, test),
                                   callback = callback,
                                   error_callback = error_callback)
    test_executors.close()
    test_executors.join()
    test_end = timeit.default_timer()
    threading_speedup = f"{test_time / (test_end - test_start) * 100:.2f}%"
    print(f"Overall time: {test_end - test_start:.2f} seconds")
    print(f"Summarized test time: {test_time:.2f} ({threading_speedup})")

    if len(failed_tests) != 0:
        print(f"Failed: {len(failed_tests)}/{len(tests)}")
        for failed_test in failed_tests:
            print(f"  {failed_test}")
