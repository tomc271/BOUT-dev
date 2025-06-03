#!/bin/bash
set -e
set -x  # Enable debug output

echo "HOME=$HOME"
echo "Checking initial state of /home/runner/local"
ls -ld /home/runner/local || echo "Directory does not exist"

if test "$BUILD_PETSC" ; then
    echo "****************************************"
    echo "Cleaning previous PETSc/SLEPc installation"
    echo "****************************************"
    rm -rf "$HOME/local/petsc" "$HOME/local/slepc"
    mkdir -p "$HOME/local/petsc" "$HOME/local/slepc"

    echo "****************************************"
    echo "Building PETSc"
    echo "****************************************"
    branch="${1:-v3.23.3}"  # Pin to tag v3.23.3
    git clone -b "$branch" https://gitlab.com/petsc/petsc.git petsc --depth=1
    cd petsc
    git log -1 --pretty=%H  # Log commit hash
    ./configure \
        --with-mpi=yes \
        --with-precision=double \
        --with-scalar-type=real \
        --with-shared-libraries=1 \
        --with-debugging=0 \
        COPTFLAGS="-O3" CXXOPTFLAGS="-O3" FOPTFLAGS="-O3" \
        --prefix="$HOME/local/petsc" \
        || { echo "PETSc configure failed"; exit 1; }
    make -j$(nproc) || { echo "PETSc make failed"; exit 1; }
    make install || { echo "PETSc install failed"; exit 1; }
    if [ ! -f "$HOME/local/petsc/include/petscmat.h" ]; then
        echo "Error: petscmat.h not found after install"
        ls -la "$HOME/local/petsc/include" || echo "Include directory missing"
        exit 1
    fi
    echo "PETSc version:"
    cat "$HOME/local/petsc/include/petscversion.h"
    echo "MatFDColoringSetFunction signature:"
    grep MatFDColoringSetFunction "$HOME/local/petsc/include/petscmat.h" || echo "Signature not found"
    cd ..

    echo "****************************************"
    echo "Building SLEPc"
    echo "****************************************"
    git clone -b "$branch" https://gitlab.com/slepc/slepc.git slepc --depth=1
    cd slepc
    git log -1 --pretty=%H
    unset SLEPC_DIR
    unset SLEPC_ARCH
    PETSC_DIR="$HOME/local/petsc" ./configure --prefix="$HOME/local/slepc" \
        || { echo "SLEPc configure failed"; exit 1; }
    make SLEPC_DIR="$(pwd)" PETSC_DIR="$HOME/local/petsc" -j$(nproc) \
        || { echo "SLEPc make failed"; exit 1; }
    make SLEPC_DIR="$(pwd)" PETSC_DIR="$HOME/local/petsc" install \
        || { echo "SLEPc install failed"; exit 1; }
    if [ ! -f "$HOME/local/slepc/lib/libslepc.so" ]; then
        echo "Error: libslepc.so not found after install"
        ls -la "$HOME/local/slepc/lib" || echo "Lib directory missing"
        exit 1
    fi
    cd ..

    echo "****************************************"
    echo "Finished building PETSc/SLEPc"
    echo "****************************************"
else
    echo "****************************************"
    echo "PETSc not requested (BUILD_PETSC=$BUILD_PETSC)"
    echo "****************************************"
    exit 1  # Fail if PETSc build is expected
fi#!/bin/bash
set -e
set -x  # Enable debug output

echo "HOME=$HOME"
echo "Checking initial state of /home/runner/local"
ls -ld /home/runner/local || echo "Directory does not exist"

if test "$BUILD_PETSC" ; then
    echo "****************************************"
    echo "Cleaning previous PETSc/SLEPc installation"
    echo "****************************************"
    rm -rf "$HOME/local/petsc" "$HOME/local/slepc"
    mkdir -p "$HOME/local/petsc" "$HOME/local/slepc"

    echo "****************************************"
    echo "Building PETSc"
    echo "****************************************"
    branch="${1:-v3.23.3}"  # Pin to tag v3.23.3
    git clone -b "$branch" https://gitlab.com/petsc/petsc.git petsc --depth=1
    cd petsc
    git log -1 --pretty=%H  # Log commit hash
    ./configure \
        --with-mpi=yes \
        --with-precision=double \
        --with-scalar-type=real \
        --with-shared-libraries=1 \
        --with-debugging=0 \
        COPTFLAGS="-O3" CXXOPTFLAGS="-O3" FOPTFLAGS="-O3" \
        --prefix="$HOME/local/petsc" \
        || { echo "PETSc configure failed"; exit 1; }
    make -j$(nproc) || { echo "PETSc make failed"; exit 1; }
    make install || { echo "PETSc install failed"; exit 1; }
    if [ ! -f "$HOME/local/petsc/include/petscmat.h" ]; then
        echo "Error: petscmat.h not found after install"
        ls -la "$HOME/local/petsc/include" || echo "Include directory missing"
        exit 1
    fi
    echo "PETSc version:"
    cat "$HOME/local/petsc/include/petscversion.h"
    echo "MatFDColoringSetFunction signature:"
    grep MatFDColoringSetFunction "$HOME/local/petsc/include/petscmat.h" || echo "Signature not found"
    cd ..

    echo "****************************************"
    echo "Building SLEPc"
    echo "****************************************"
    git clone -b "$branch" https://gitlab.com/slepc/slepc.git slepc --depth=1
    cd slepc
    git log -1 --pretty=%H
    unset SLEPC_DIR
    unset SLEPC_ARCH
    PETSC_DIR="$HOME/local/petsc" ./configure --prefix="$HOME/local/slepc" \
        || { echo "SLEPc configure failed"; exit 1; }
    make SLEPC_DIR="$(pwd)" PETSC_DIR="$HOME/local/petsc" -j$(nproc) \
        || { echo "SLEPc make failed"; exit 1; }
    make SLEPC_DIR="$(pwd)" PETSC_DIR="$HOME/local/petsc" install \
        || { echo "SLEPc install failed"; exit 1; }
    if [ ! -f "$HOME/local/slepc/lib/libslepc.so" ]; then
        echo "Error: libslepc.so not found after install"
        ls -la "$HOME/local/slepc/lib" || echo "Lib directory missing"
        exit 1
    fi
    cd ..

    echo "****************************************"
    echo "Finished building PETSc/SLEPc"
    echo "****************************************"
else
    echo "****************************************"
    echo "PETSc not requested (BUILD_PETSC=$BUILD_PETSC)"
    echo "****************************************"
    exit 1  # Fail if PETSc build is expected
fi