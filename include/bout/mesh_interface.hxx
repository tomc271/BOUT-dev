
#ifndef BOUT_MESH_INTERFACE_HXX
#define BOUT_MESH_INTERFACE_HXX

#include "coordinates.hxx"
#include "fieldgroup.hxx"
#include "bout/field2d.hxx"
#include "bout/field3d.hxx"
#include "bout/field_data.hxx"

/// Type used to return pointers to handles
using comm_handle = void*;

class MeshInterface {

public:
  /// Perform communications without waiting for them
  /// to finish. Requires a call to wait() afterwards.
  ///
  /// \param g Group of fields to communicate
  /// \returns handle to be used as input to wait()
  virtual comm_handle send(FieldGroup& g) = 0;

  /// Send only the x-guard cells
  virtual comm_handle sendX(FieldGroup& g, comm_handle handle, bool disable_corners) = 0;

  /// Send only the y-guard cells
  virtual comm_handle sendY(FieldGroup& g, comm_handle handle) = 0;
};

#endif //BOUT_MESH_INTERFACE_HXX
