{ pkgs ? import <nixpkgs> {} }:

let
  pythonVersion = "3.12"; # Specify Python version
  pythonPackages = pkgs.python312Packages; # Use Python 3.12 packages
in
pkgs.mkShell {
  # Define build inputs (dependencies)
  buildInputs = [
    pkgs.gcc                     # Compiler
    pkgs.stdenv.cc.cc.lib        # Ensure libstdc++.so.6 is available
    pkgs.zlib                    # Add zlib (provides libz.so.1)
    pkgs.raylib                  # Raylib library
    pkgs.xorg.libX11             # X11 library
    pkgs.mesa.drivers            # OpenGL library (includes libGL)
    pkgs.python312               # Python 3.12 interpreter
    pkgs.glibc                   # GNU C Library
  ];

  # Add Python packages to the environment
  nativeBuildInputs = [
    pythonPackages.virtualenv     # Virtualenv for creating .venv
    pythonPackages.pip            # Pip for installing Python packages
    pythonPackages.setuptools     # Setuptools for package management
  ];

  # Set environment variables
  shellHook = ''
    # Ensure Python 3.12 is used
    export PATH="${pkgs.python312}/bin:$PATH"

    # Ensure libstdc++.so.6 and libz.so.1 are in the library path
    export LD_LIBRARY_PATH="${pkgs.stdenv.cc.cc.lib}/lib:${pkgs.zlib}/lib:$LD_LIBRARY_PATH"

    # Create a virtual environment if it doesn't exist
    if [ ! -d ".venv" ]; then
      echo "Creating virtual environment in .venv..."
      virtualenv --python=${pkgs.python312}/bin/python3 .venv
    fi

    # Activate the virtual environment
    if [ -f ".venv/bin/activate" ]; then
      source .venv/bin/activate
    else
      echo "Error: Failed to create or activate virtual environment."
      exit 1
    fi

    # Install required Python packages into the virtual environment
    if [ ! -f ".venv/.installed" ]; then
      echo "Installing Python dependencies..."
      pip install numpy geopandas
      touch .venv/.installed
    fi

    # Provide a helpful message when entering the shell
    echo "Environment ready for building your project."
    echo "Python 3.12 virtual environment is activated."
    echo "Run 'python' to start the interpreter."
  '';
}