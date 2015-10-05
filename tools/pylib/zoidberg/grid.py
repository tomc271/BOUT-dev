from __future__ import division
from builtins import object
from past.utils import old_div

import numpy as np

class Grid(object):
    def __init__(self, nx, ny, nz,
                 Lx=0.1, Ly=10., Lz = 1.,
                 name="fci_grid"):
        """Initialise a grid object

        Inputs
        ------
        nx  - Number of radial points
        ny  - Number of toroidal points (NOTE: Different to BOUT++ standard)
        nz  - Number of poloidal points

        Lx  - Radial domain size  [m]
        Ly  - Toroidal domain size [m]
        Lz  - Poloidal domain size [m]

        name - Grid name (default "fci_grid")
        """

        self.name = name

        self.nx = int(nx)
        self.ny = int(ny)
        self.nz = int(nz)

        self.Lx = float(Lx)
        self.Ly = float(Ly)
        self.Lz = float(Lz)

        self.delta_x = old_div(Lx,nx)
        self.delta_y = old_div(Ly,ny)
        self.delta_z = old_div(Lz,nz)

        # Coord arrays
        self.xarray = np.linspace(0,Lx,nx)
        self.yarray = np.linspace(0,Ly,ny)
        self.zarray = np.linspace(0,Lz,nz,endpoint=False)

        self.xcentre = 0.5*max(self.xarray)
        self.zcentre = 0.5*max(self.zarray)

        # How to do this properly?
        self.Rmaj = 1.0
