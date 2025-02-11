{ pkgs ? import <nixpkgs> {} }:

let
  pythonVersion = "3.12";
  pythonPackages = pkgs.python312Packages;
in
pkgs.mkShell {
  buildInputs = [
    # Base development tools
    pkgs.gcc
    pkgs.stdenv.cc.cc.lib
    pkgs.zlib
    pkgs.raylib
    pkgs.xorg.libX11
    pkgs.mesa.drivers
    pkgs.python312
    pkgs.glibc

    # Emscripten and web development tools
    pkgs.emscripten
    pkgs.llvmPackages.clang
    pkgs.cmake
    pkgs.nodejs
    pkgs.binaryen
    pkgs.git
    pkgs.pkg-config
  ];

  nativeBuildInputs = [
    pythonPackages.virtualenv
    pythonPackages.pip
    pythonPackages.setuptools
  ];

  shellHook = ''
    # Basic environment setup
    export PATH="${pkgs.python312}/bin:$PATH"
    export LD_LIBRARY_PATH="${pkgs.stdenv.cc.cc.lib}/lib:${pkgs.zlib}/lib:$LD_LIBRARY_PATH"

    # Emscripten setup
    export EMSDK=${pkgs.emscripten}
    export EMSCRIPTEN=${pkgs.emscripten}/share/emscripten
    export EMSCRIPTEN_ROOT=${pkgs.emscripten}/share/emscripten
    export LLVM_ROOT=${pkgs.llvmPackages.clang}/bin
    export BINARYEN_ROOT=${pkgs.binaryen}
    export PATH="$EMSCRIPTEN:$LLVM_ROOT:$PATH"

    # Create or update Emscripten config
    cat > "$HOME/.emscripten" << EOF
    import os
    LLVM_ROOT = '${pkgs.llvmPackages.clang}/bin'
    EMSCRIPTEN_ROOT = '${pkgs.emscripten}/share/emscripten'
    BINARYEN_ROOT = '${pkgs.binaryen}'
    NODE_JS = '${pkgs.nodejs}/bin/node'
    PYTHON = '${pkgs.python312}/bin/python3'
    CACHE = os.path.expanduser('~/.emscripten_cache')
    PORTS = os.path.expanduser('~/.emscripten_ports')
    EOF

    # Create cache directory if it doesn't exist
    mkdir -p ~/.emscripten_cache

    # Python virtual environment setup
    if [ ! -d ".venv" ]; then
      echo "Creating virtual environment in .venv..."
      virtualenv --python=${pkgs.python312}/bin/python3 .venv
    fi

    if [ -f ".venv/bin/activate" ]; then
      source .venv/bin/activate
    else
      echo "Error: Failed to create or activate virtual environment."
      exit 1
    fi

    if [ ! -f ".venv/.installed" ]; then
      echo "Installing Python dependencies..."
      pip install numpy geopandas
      touch .venv/.installed
    fi

    # Environment information
    echo "Environment ready for building your project."
    echo "Python 3.12 virtual environment is activated."
    echo "Emscripten is configured and ready to use."
    echo "Run 'emcc --version' to verify Emscripten installation."
  '';
}