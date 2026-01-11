import os
import subprocess
import sys


# Paths for project, problems sources files and the build files with the executables
current_file = os.path.abspath(__file__)
root_dir = os.path.dirname(os.path.dirname(current_file))

build_dir = os.path.join(root_dir, "build")

#problems_build_dir = os.path.join(build_dir, "CatMADS/problems")
problems_build_dir = os.path.join(build_dir, "CatMADS/problems/unconstrained")


# Build main NOMAD files
def build_root_project():
    if not os.path.exists(build_dir) or not os.listdir(build_dir):
        print("Building the root project...")
        subprocess.run(
            ["cmake", "-S", root_dir, "-B", build_dir],
            cwd=root_dir,
            check=True,
        )
        subprocess.run(
            ["cmake", "--build", build_dir],
            cwd=build_dir,
            check=True,
        )
        print("Root project built successfully.")
    else:
        print("Root project is already built. Skipping root build.")

# To run the optimization on different OS
def get_executable_name(problem):
    if platform.system() == "Windows":
        return f"{problem}.exe"
    else:
        return problem  # No extension on Unix-based systems

# Build-and-run a problem
def run_problem_executable(problem):
    # Construct the path to the executable
    executable = os.path.join(problems_build_dir, problem, f"{problem}.exe")
    if os.path.exists(executable):
        print(f"Running {problem}...")
        try:
            # Use subprocess.Popen to stream output in real time
            with subprocess.Popen(
                [executable],
                cwd=os.path.dirname(executable),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            ) as process:
                for line in process.stdout:
                    print(line, end="")  # Print stdout as it arrives
                for line in process.stderr:
                    print(line, end="")  # Print stderr as it arrives
                return_code = process.wait()
                if return_code != 0:
                    print(f"{problem} exited with code {return_code}.")
        except subprocess.CalledProcessError as e:
            print(f"Error while running {problem}: {e}")
    else:
        print(f"Executable for {problem} not found at {executable}!")


if __name__ == '__main__':

    #problems = ["AckleyHard", "Beale", "Beale_constrained", "Branin", "Branin_constrained", "Bukin6", "Bukin6_constrained",
    #"Dembo5_constrained", "EDV2", "EDV2_constrained", "G09_constrained", "GoldsteinPrice_constrained",
    #"GoldsteinPrice1", "GoldsteinPrice2", "Himmelblau_constrained", "HS78", "HS144_constrained", "Pentagon_constrained",
    #"PressureVessel_constrained", "Rastragin", "ReinforcedConcreteBeam_constrained", "Rosenbrock_constrained",
    #"RosenbrockMixed", "RosenSuzuki", "StyblinskiTang", "StyblinskiTang_constrained", "Toy1", "Toy2",
    #"Toy_constrained", "Wong1", "Wong2_constrained", "Zakharov"]


    problems = ["Camel", "EVD61", "Gamma", "Hal04", "Hartmann", "Ishigami",
                "KowalikOsborne", "Levy", "McCormick", "OET5",
                "Roustant", "Shekel", "ThreeHump", "Wong3"]


    # 1) Delete/clean build with 
    # "rm -rf ~/nomad4dev/build" in terminal

    # 2) Build NOMAD
    build_root_project()

    # 3) Build-and-run problem specific files
    for problem in problems:
        try:
            print(f"Processing {problem}...")
            run_problem_executable(problem)
        except FileNotFoundError as e:
            print(e)
        print("-" * 40)
