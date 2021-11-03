Hasegawa-Wakatani with RF external boundary
===========================================

The Hasegawa-Wakatani equations, implemented using Poisson brackets `[]`:

    ddt(n) = -[phi, n] + alpha*(phi - n) - kappa*DDZ(phi)
    ddt(vorticity) = -[phi, vorticity] + alpha*(phi - n)

where `alpha`, `kappa` are constants


Setting outer phi boundary
--------------------------

To set the outer boundary, a tabulated input file can be used. The option
`hw:phi_boundary_file` can be set to the name of a file to read. This can be
specified in the BOUT.inp file e.g.
```
[hw]
phi_boundary_file = phi_boundary.txt
```
or on the command line
```
$ ./hasegawa-wakatani-rf hw:phi_boundary_file=phi_boundary.txt
```

The text file should contain values of potential in a column:
```
0.1
0.2
...
```

The number of values does not have to be the same as the resolution used in
the simulation; the values are assumed to be evenly spaced in the periodic Z domain
(not including end point i.e. the last point is not the same as the first),
and linearly interpolated onto the grid.
