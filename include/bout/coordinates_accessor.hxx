#pragma once
#ifndef COORDINATES_ACCESSOR_H__
#define COORDINATES_ACCESSOR_H__

#include "array.hxx"
#include "build_config.hxx"
#include "coordinates.hxx"

/// Provide (hopefully) fast access to Coordinates data
/// e.g. grid spacing, metric tensors etc.
///
/// Contains only two member variables:
///   - data     a BoutReal pointer
///   - mesh_nz  an int
///
/// data is striped, so that quantities at a given
/// grid cell are packed together.
///
/// Example
///
///   auto coord_acc = CoordinatesAccessor(mesh->getCoordinates());
///   coord_acc.dx(index)  -> BoutReal at cell index
///
/// Notes
///
///  * Data from Coordinates is copied into an array which
///    is cached. CoordinatesAccessors created with the same
///    Coordinates pointer will re-use the same array without
///    copying the data again.
///    -> If Coordinates data is changed, the cache should be cleared
///    by calling CoordinatesAccessor::clear()
struct CoordinatesAccessor {
  CoordinatesAccessor() = default;

  /// Constructor from Coordinates
  /// Copies data from coords, doesn't modify it
  explicit CoordinatesAccessor(const Coordinates* coords);

  /// Clear the cache of Coordinates data
  ///
  /// By default this clears everything; if only a specific
  /// Coordinates should be removed then that can be specified.
  ///
  /// Returns the number of data arrays removed
  /// (mainly to assist in testing)
  static std::size_t clear(const Coordinates* coords = nullptr);

  /// Offsets of each coordinates variable into the striped array
  enum class Offset {
    dx,
    dy,
    dz, // Grid spacing
    d1_dx,
    d1_dy,
    d1_dz, // Grid spacing non-uniformity
    J,     // Jacobian
    B,
    Byup,
    Bydown, // Magnetic field magnitude
    G1,
    G3, // Metric derivatives
    g11,
    g12,
    g13,
    g22,
    g23,
    g33, // Contravariant metric tensor (g^{ij})
    g_11,
    g_12,
    g_13,
    g_22,
    g_23,
    g_33, // Covariant metric tensor
    end
  };

  /// The number of values for each grid point
  /// Note: This might be > end to align to memory boundaries
  ///       e.g. 32-byte boundaries -> multiples of 32 / 8 = 4 desirable
  static constexpr int stripe_size = static_cast<int>(Offset::end);

  static_assert(stripe_size >= static_cast<int>(Offset::end),
                "stripe_size must fit all Coordinates values");

  /// Underlying data pointer.
  /// This array includes all Coordinates fields interleaved
  BoutReal* data{nullptr};
  int mesh_nz{0}; ///< For converting from 3D to 2D index

  /// Lookup value in data array, based on the cell index
  /// and the variable offset
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal lookup(int index, Offset offset) const {
#if BOUT_USE_METRIC_3D
    const int ind = index; // Use 3D index
#else
    const int ind = index / mesh_nz; // Convert to a 2D index
#endif
    return data[(stripe_size * ind) + static_cast<int>(offset)];
  }

  // A type-safe templated lookup method using enum classes
  template <Offset O>
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal get_metric(int index) const {
    return lookup(index, O);
  }

  [[nodiscard]] BOUT_HOST_DEVICE BoutReal dx(int idx) const {
    return get_metric<Offset::dx>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal dy(int idx) const {
    return get_metric<Offset::dy>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal dz(int idx) const {
    return get_metric<Offset::dz>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal d1_dx(int idx) const {
    return get_metric<Offset::d1_dx>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal d1_dy(int idx) const {
    return get_metric<Offset::d1_dy>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal d1_dz(int idx) const {
    return get_metric<Offset::d1_dz>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal J(int idx) const {
    return get_metric<Offset::J>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal B(int idx) const {
    return get_metric<Offset::B>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal Byup(int idx) const {
    return get_metric<Offset::Byup>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal Bydown(int idx) const {
    return get_metric<Offset::Bydown>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal G1(int idx) const {
    return get_metric<Offset::G1>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal G3(int idx) const {
    return get_metric<Offset::G3>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g11(int idx) const {
    return get_metric<Offset::g11>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g12(int idx) const {
    return get_metric<Offset::g12>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g13(int idx) const {
    return get_metric<Offset::g13>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g22(int idx) const {
    return get_metric<Offset::g22>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g23(int idx) const {
    return get_metric<Offset::g23>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g33(int idx) const {
    return get_metric<Offset::g33>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g_11(int idx) const {
    return get_metric<Offset::g_11>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g_12(int idx) const {
    return get_metric<Offset::g_12>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g_13(int idx) const {
    return get_metric<Offset::g_13>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g_22(int idx) const {
    return get_metric<Offset::g_22>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g_23(int idx) const {
    return get_metric<Offset::g_23>(idx);
  }
  [[nodiscard]] BOUT_HOST_DEVICE BoutReal g_33(int idx) const {
    return get_metric<Offset::g_33>(idx);
  }
};

#endif // COORDINATES_ACCESSOR_H__
