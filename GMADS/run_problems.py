import os
import subprocess
import sys


# Paths for project, problems sources files and the build files with the executables
current_file = os.path.abspath(__file__)
root_dir = os.path.dirname(os.path.dirname(current_file))

build_dir = os.path.join(root_dir, "build")

#problems_build_dir = os.path.join(build_dir, "CatMADS/problems")


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


    # Old constrained and unconstrained pbs
    #problems = ["AckleyHard", "Beale", "Beale_constrained", "Branin", "Branin_constrained", "Bukin6", "Bukin6_constrained",
    #"Dembo5_constrained", "EDV2", "EDV2_constrained", "G09_constrained", "GoldsteinPrice_constrained",
    #"GoldsteinPrice1", "GoldsteinPrice2", "Himmelblau_constrained", "HS78", "HS144_constrained", "Pentagon_constrained",
    #"PressureVessel_constrained", "Rastragin", "ReinforcedConcreteBeam_constrained", "Rosenbrock_constrained",
    #"RosenbrockMixed", "RosenSuzuki", "StyblinskiTang", "StyblinskiTang_constrained", "Toy1", "Toy2",
    #"Toy_constrained", "Wong1", "Wong2_constrained", "Zakharov"]

    # New unconstrained pbs
    #problems = ["Camel", "EVD61", "Gamma", "Hal04", "Hartmann", "Ishigami",
    #            "KowalikOsborne", "Levy", "McCormick", "OET5",
    #            "Roustant", "Shekel", "ThreeHump", "Wong3"]


    # New constrained pbs
    #problems = ["SpeedReducer_constrained", "Spring_constrained", "G07_constrained",
    #            "CarSideImpact_constrained", "Dembo7_constrained", "MAD_constrained",
    #            "Wong3_constrained", "Welded_beam_constrained", "Three_bar_truss_constrained",
    #            "Three_humps_constrained", "McCormick_constrained", "G06_constrained",
    #            "Shekel_constrained", "Ishigami_constrained"]


    #problems = ["Beale_constrained"]


    # Unconstrained for G-MADS vs CatMADS
    #problems = ["GoldsteinPrice2", "Ishigami", "Hartmann", "Levy", "Camel", "Gamma", "EVD61", "Hal04",
    #            "OET5", "Wong3", "Roustant", "KowalikOsborne", "ThreeHump", "McCormick", "Shekel"]      # 15 pbs
    #problems = ["Ishigami_GMADS", "Hartmann_GMADS", "Levy_GMADS", "Camel_GMADS", "Gamma_GMADS", "EVD61_GMADS",
    #            "KowalikOsborne_GMADS", "ThreeHumps_GMADS", "McCormick_GMADS", "Shekel_GMADS"]






    # Constrained for G-MADS vs CatMADS
    problems = ["CarSideImpact_constrained_GMADS", "WeldedBeam_constrained_GMADS", "G06_constrained_GMADS", "SpeedReducer_constrained_GMADS",
                "G07_constrained_GMADS", "McCormick_constrained_GMADS", "Ishigami_constrained_GMADS",
                "ThreeHumps_constrained_GMADS", "Shekel_constrained_GMADS", "Spring_constrained_GMADS"]


    # All unconstrained pbs
    #problems = ["AckleyHard_GMADS", "Beale_GMADS", "Branin_GMADS",  "Bukin6_GMADS", "EVD2_GMADS", "GoldsteinPrice1_GMADS",
    # "GoldsteinPrice2_GMADS", "HS78_GMADS", "Rastragin_GMADS", "RosenbrockMixed_GMADS", "RosenSuzuki_GMADS",
    # "StyblinskiTang_GMADS", "Toy1_GMADS", "Toy2_GMADS", "Wong1_GMADS",  "Zakharov_GMADS",
    # "Camel_GMADS", "EVD61_GMADS", "Gamma_GMADS", "Hal04_GMADS", "Hartmann_GMADS", "Ishigami_GMADS",
    # "KowalikOsborne_GMADS", "Levy_GMADS", "McCormick_GMADS", "OET5_GMADS", "Roustant_GMADS", "Shekel_GMADS",
    # "ThreeHumps_GMADS", "Wong3_GMADS"]



    # 0) Proper directory with problems
    problems_build_dir = os.path.join(build_dir, "GMADS/problems/constrained")


    # 1) Delete/clean build with 
    #"rm -rf ~/nomad4dev/build" in terminal

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
