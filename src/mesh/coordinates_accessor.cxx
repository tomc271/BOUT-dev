#include "bout/coordinates_accessor.hxx"
#include "bout/build_defines.hxx"
#include "bout/mesh.hxx"

#include <map>

namespace {
/// Associate each Coordinates object with an Array object
/// which contains the coordinates data in striped form.
///
/// Note: This association could perhaps be done by putting
///       the Array inside Coordinates, but this keeps things decoupled
std::map<const Coordinates*, Array<BoutReal>> coords_store;
} // namespace

CoordinatesAccessor::CoordinatesAccessor(const Coordinates* coords) {
  ASSERT0(coords != nullptr);

  // Size of the mesh in Z. Used to convert 3D -> 2D index
  Mesh* mesh = coords->dx.getMesh();
  mesh_nz = mesh->LocalNz;

  auto search = coords_store.find(coords);
  if (search != coords_store.end()) {
    // Found, so get the pointer to the data
    data = search->second.begin();
    return;
  }

  // Not yet created, so create the array and copy the data

  // Work out the size needed
  int array_size = stripe_size * mesh->LocalNx * mesh->LocalNy;
#if BOUT_USE_METRIC_3D
  array_size *= mesh->LocalNz; // 3D metrics
#endif

  // Create the array and get the underlying data
  data = coords_store.emplace(coords, array_size).first->second.begin();

  // Iterate over all points in the field
  // Note this could be 2D or 3D, depending on FieldMetric type
  for (const auto& ind : coords->dx.getRegion("RGN_ALL")) {

    auto copy_stripe = [&](auto mem_ptr, Offset offset) {
      if ((coords->*mem_ptr).isAllocated()) {
        data[stripe_size * ind.ind + static_cast<int>(offset)] = (coords->*mem_ptr)[ind];
      }
    };

    copy_stripe(&Coordinates::dx, Offset::dx);
    copy_stripe(&Coordinates::dy, Offset::dy);
    copy_stripe(&Coordinates::dz, Offset::dz);

    copy_stripe(&Coordinates::d1_dx, Offset::d1_dx);
    copy_stripe(&Coordinates::d1_dy, Offset::d1_dy);
    copy_stripe(&Coordinates::d1_dz, Offset::d1_dz);

    copy_stripe(&Coordinates::J, Offset::J);
    copy_stripe(&Coordinates::G1, Offset::G1);
    copy_stripe(&Coordinates::G3, Offset::G3);

    copy_stripe(&Coordinates::g11, Offset::g11);
    copy_stripe(&Coordinates::g12, Offset::g12);
    copy_stripe(&Coordinates::g13, Offset::g13);
    copy_stripe(&Coordinates::g22, Offset::g22);
    copy_stripe(&Coordinates::g23, Offset::g23);
    copy_stripe(&Coordinates::g33, Offset::g33);

    copy_stripe(&Coordinates::g_11, Offset::g_11);
    copy_stripe(&Coordinates::g_12, Offset::g_12);
    copy_stripe(&Coordinates::g_13, Offset::g_13);
    copy_stripe(&Coordinates::g_22, Offset::g_22);
    copy_stripe(&Coordinates::g_23, Offset::g_23);
    copy_stripe(&Coordinates::g_33, Offset::g_33);

    // Bxy handling requires nested member evaluation, so we keep it explicit
    if (coords->Bxy.isAllocated()) {
      data[stripe_size * ind.ind + static_cast<int>(Offset::B)] = coords->Bxy[ind];
      if (coords->Bxy.yup().isAllocated()) {
        data[stripe_size * ind.ind + static_cast<int>(Offset::Byup)] =
            coords->Bxy.yup()[ind];
      }
      if (coords->Bxy.ydown().isAllocated()) {
        data[stripe_size * ind.ind + static_cast<int>(Offset::Bydown)] =
            coords->Bxy.ydown()[ind];
      }
    }
  }
}

std::size_t CoordinatesAccessor::clear(const Coordinates* coords) {
  if (coords == nullptr) {
    // clear all
    std::size_t num_removed = coords_store.size();
    coords_store.clear();
    return num_removed;
  }
  // Coordinates specified, so only remove one
  return coords_store.erase(coords);
}
