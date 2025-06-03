#!/bin/bash
set -e

if test $BUILD_PETSC ; then
    echo "****************************************"
    echo "Cleaning previous PETSc/SLEPc installation"
    echo "****************************************"
    rm -rf $HOME/local/petsc $HOME/local/slepc

    echo "****************************************"
    echo "Building PETSc"
    echo "****************************************"
    branch=${1:-v3.23.3}  # Pin to specific tag
    git clone -b $branch https://gitlab.com/petsc/petsc.git petsc --depth=1
    cd petsc
    git log -1 --pretty=%H  # Log commit hash
    ./configure \
        --with-mpi=yes \
        --with-precision=double \
        --with-scalar-type=real \
        --with-shared-libraries=1 \
        --with-debugging=0 \
        {C,CXX,F}OPTFLAGS="-O3" \
        --prefix=$HOME/local/petsc
    make
    make install
    if [ ! -f $HOME/local/petsc/include/petscmat.h ]; then
        echo "Error: petscmat.h not found after install"
        exit 1
    fi
    echo "PETSc version:"
    cat $HOME/local/petsc/include/petscversion.h
    echo "MatFDColoringSetFunction signature:"
    grep MatFDColoringSetFunction $HOME/local/petsc/include/petscmat.h
    cd ..

    echo "****************************************"
    echo "Building SLEPc"
    echo "****************************************"
    git clone -b $branch https://gitlab.com/slepc/slepc.git slepc --depth=1
    cd slepc
    git log -1 --pretty=%H
    unset SLEPC_DIR
    unset SLEPC_ARCH
    PETSC_DIR=$HOME/local/petsc ./configure --prefix=$HOME/local/slepc
    make SLEPC_DIR=$(pwd) PETSC_DIR=$HOME/local/petsc
    make SLEPC_DIR=$(pwd) PETSC_DIR=$HOME/local/petsc install
    if [ ! -f $HOME/local/slepc/lib/libslepc.so ]; then
        echo "Error: libslepc.so not found after install"
        exit 1
    fi
    cd ..

    echo "****************************************"
    echo "Finished building PETSc/SLEPc"
    echo "****************************************"
else
    echo "****************************************"
    echo "PETSc not requested"
    echo "****************************************"
fi