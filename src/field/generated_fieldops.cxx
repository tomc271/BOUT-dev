// This file is autogenerated - see gen_fieldops.py
#include <bout/mesh.hxx>
#include <bout/region.hxx>

#include <field2d.hxx>
#include <field3d.hxx>
#include <globals.hxx>
#include <interpolation.hxx>
#include <bout/scorepwrapper.hxx>

// Provide the C++ wrapper for multiplication of Field3D and Field3D
Field3D operator*(const Field3D &lhs, const Field3D &rhs) {
  SCOREP0();
#if CHECK > 0
  if (lhs.getLocation() != rhs.getLocation()) {
    throw BoutException("Error in operator*(Field3D, Field3D): fields at different "
                        "locations. lhs is at %s, rhs is at %s!",
                        strLocation(lhs.getLocation()), strLocation(rhs.getLocation()));
  }
#endif

  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs[index] * rhs[index];);

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by multiplication with Field3D
Field3D &Field3D::operator*=(const Field3D &rhs) {
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  SCOREP0();
  if (data.unique()) {
#if CHECK > 0
    if (this->getLocation() != rhs.getLocation()) {
      throw BoutException("Error in Field3D::operator*=(Field3D): fields at different "
                          "locations. lhs is at %s, rhs is at %s!",
                          strLocation(this->getLocation()),
                          strLocation(rhs.getLocation()));
    }
#endif

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion3D("RGN_ALL"), index,
                      (*this)[index] *= rhs[index];);

    checkData(*this);

  } else {
    (*this) = (*this) * rhs;
  }
  return *this;
}

// Provide the C++ wrapper for division of Field3D and Field3D
Field3D operator/(const Field3D &lhs, const Field3D &rhs) {
  SCOREP0();
#if CHECK > 0
  if (lhs.getLocation() != rhs.getLocation()) {
    throw BoutException("Error in operator/(Field3D, Field3D): fields at different "
                        "locations. lhs is at %s, rhs is at %s!",
                        strLocation(lhs.getLocation()), strLocation(rhs.getLocation()));
  }
#endif

  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs[index] / rhs[index];);

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by division with Field3D
Field3D &Field3D::operator/=(const Field3D &rhs) {
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  SCOREP0();
  if (data.unique()) {
#if CHECK > 0
    if (this->getLocation() != rhs.getLocation()) {
      throw BoutException("Error in Field3D::operator/=(Field3D): fields at different "
                          "locations. lhs is at %s, rhs is at %s!",
                          strLocation(this->getLocation()),
                          strLocation(rhs.getLocation()));
    }
#endif

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion3D("RGN_ALL"), index,
                      (*this)[index] /= rhs[index];);

    checkData(*this);

  } else {
    (*this) = (*this) / rhs;
  }
  return *this;
}

// Provide the C++ wrapper for addition of Field3D and Field3D
Field3D operator+(const Field3D &lhs, const Field3D &rhs) {
  SCOREP0();
#if CHECK > 0
  if (lhs.getLocation() != rhs.getLocation()) {
    throw BoutException("Error in operator+(Field3D, Field3D): fields at different "
                        "locations. lhs is at %s, rhs is at %s!",
                        strLocation(lhs.getLocation()), strLocation(rhs.getLocation()));
  }
#endif

  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs[index] + rhs[index];);

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by addition with Field3D
Field3D &Field3D::operator+=(const Field3D &rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {
#if CHECK > 0
    if (this->getLocation() != rhs.getLocation()) {
      throw BoutException("Error in Field3D::operator+=(Field3D): fields at different "
                          "locations. lhs is at %s, rhs is at %s!",
                          strLocation(this->getLocation()),
                          strLocation(rhs.getLocation()));
    }
#endif

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion3D("RGN_ALL"), index,
                      (*this)[index] += rhs[index];);

    checkData(*this);

  } else {
    (*this) = (*this) + rhs;
  }
  return *this;
}

// Provide the C++ wrapper for subtraction of Field3D and Field3D
Field3D operator-(const Field3D &lhs, const Field3D &rhs) {
  SCOREP0();
#if CHECK > 0
  if (lhs.getLocation() != rhs.getLocation()) {
    throw BoutException("Error in operator-(Field3D, Field3D): fields at different "
                        "locations. lhs is at %s, rhs is at %s!",
                        strLocation(lhs.getLocation()), strLocation(rhs.getLocation()));
  }
#endif

  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs[index] - rhs[index];);

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by subtraction with Field3D
Field3D &Field3D::operator-=(const Field3D &rhs) {
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  SCOREP0();
  if (data.unique()) {
#if CHECK > 0
    if (this->getLocation() != rhs.getLocation()) {
      throw BoutException("Error in Field3D::operator-=(Field3D): fields at different "
                          "locations. lhs is at %s, rhs is at %s!",
                          strLocation(this->getLocation()),
                          strLocation(rhs.getLocation()));
    }
#endif

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion3D("RGN_ALL"), index,
                      (*this)[index] -= rhs[index];);

    checkData(*this);

  } else {
    (*this) = (*this) - rhs;
  }
  return *this;
}

// Provide the C++ wrapper for multiplication of Field3D and Field2D
Field3D operator*(const Field3D &lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    const auto base_ind = localmesh->ind2Dto3D(index);
                    for (int jz = 0; jz < localmesh->LocalNz; ++jz) {
                      result[base_ind + jz] = lhs[base_ind + jz] * rhs[index];
                    });

  result.setLocation(lhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by multiplication with Field2D
Field3D &Field3D::operator*=(const Field2D &rhs) {
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  SCOREP0();
  if (data.unique()) {

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index,
                      const auto base_ind = fieldmesh->ind2Dto3D(index);
                      for (int jz = 0; jz < fieldmesh->LocalNz;
                           ++jz) { (*this)[base_ind + jz] *= rhs[index]; });

    checkData(*this);

  } else {
    (*this) = (*this) * rhs;
  }
  return *this;
}

// Provide the C++ wrapper for division of Field3D and Field2D
Field3D operator/(const Field3D &lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    const auto base_ind = localmesh->ind2Dto3D(index);
                    const auto tmp = 1.0 / rhs[index];
                    for (int jz = 0; jz < localmesh->LocalNz;
                         ++jz) { result[base_ind + jz] = lhs[base_ind + jz] * tmp; });

  result.setLocation(lhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by division with Field2D
Field3D &Field3D::operator/=(const Field2D &rhs) {
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index,
                      const auto base_ind = fieldmesh->ind2Dto3D(index);
                      const auto tmp = 1.0 / rhs[index];
                      for (int jz = 0; jz < fieldmesh->LocalNz;
                           ++jz) { (*this)[base_ind + jz] *= tmp; });

    checkData(*this);

  } else {
    (*this) = (*this) / rhs;
  }
  return *this;
}

// Provide the C++ wrapper for addition of Field3D and Field2D
Field3D operator+(const Field3D &lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    const auto base_ind = localmesh->ind2Dto3D(index);
                    for (int jz = 0; jz < localmesh->LocalNz; ++jz) {
                      result[base_ind + jz] = lhs[base_ind + jz] + rhs[index];
                    });

  result.setLocation(lhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by addition with Field2D
Field3D &Field3D::operator+=(const Field2D &rhs) {
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  SCOREP0();
  if (data.unique()) {

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index,
                      const auto base_ind = fieldmesh->ind2Dto3D(index);
                      for (int jz = 0; jz < fieldmesh->LocalNz;
                           ++jz) { (*this)[base_ind + jz] += rhs[index]; });

    checkData(*this);

  } else {
    (*this) = (*this) + rhs;
  }
  return *this;
}

// Provide the C++ wrapper for subtraction of Field3D and Field2D
Field3D operator-(const Field3D &lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    const auto base_ind = localmesh->ind2Dto3D(index);
                    for (int jz = 0; jz < localmesh->LocalNz; ++jz) {
                      result[base_ind + jz] = lhs[base_ind + jz] - rhs[index];
                    });

  result.setLocation(lhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by subtraction with Field2D
Field3D &Field3D::operator-=(const Field2D &rhs) {
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  SCOREP0();
  if (data.unique()) {

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index,
                      const auto base_ind = fieldmesh->ind2Dto3D(index);
                      for (int jz = 0; jz < fieldmesh->LocalNz;
                           ++jz) { (*this)[base_ind + jz] -= rhs[index]; });

    checkData(*this);

  } else {
    (*this) = (*this) - rhs;
  }
  return *this;
}

// Provide the C++ wrapper for multiplication of Field3D and BoutReal
Field3D operator*(const Field3D &lhs, const BoutReal rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs[index] * rhs;);

  result.setLocation(lhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by multiplication with BoutReal
Field3D &Field3D::operator*=(const BoutReal rhs) {
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  SCOREP0();
  if (data.unique()) {

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion3D("RGN_ALL"), index, (*this)[index] *= rhs;);

    checkData(*this);

  } else {
    (*this) = (*this) * rhs;
  }
  return *this;
}

// Provide the C++ wrapper for division of Field3D and BoutReal
Field3D operator/(const Field3D &lhs, const BoutReal rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs[index] / rhs;);

  result.setLocation(lhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by division with BoutReal
Field3D &Field3D::operator/=(const BoutReal rhs) {
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  SCOREP0();
  if (data.unique()) {

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion3D("RGN_ALL"), index, (*this)[index] /= rhs;);

    checkData(*this);

  } else {
    (*this) = (*this) / rhs;
  }
  return *this;
}

// Provide the C++ wrapper for addition of Field3D and BoutReal
Field3D operator+(const Field3D &lhs, const BoutReal rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs[index] + rhs;);

  result.setLocation(lhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by addition with BoutReal
Field3D &Field3D::operator+=(const BoutReal rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion3D("RGN_ALL"), index, (*this)[index] += rhs;);

    checkData(*this);

  } else {
    (*this) = (*this) + rhs;
  }
  return *this;
}

// Provide the C++ wrapper for subtraction of Field3D and BoutReal
Field3D operator-(const Field3D &lhs, const BoutReal rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs[index] - rhs;);

  result.setLocation(lhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field3D by subtraction with BoutReal
Field3D &Field3D::operator-=(const BoutReal rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion3D("RGN_ALL"), index, (*this)[index] -= rhs;);

    checkData(*this);

  } else {
    (*this) = (*this) - rhs;
  }
  return *this;
}

// Provide the C++ wrapper for multiplication of Field2D and Field3D
Field3D operator*(const Field2D &lhs, const Field3D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    const auto base_ind = localmesh->ind2Dto3D(index);
                    for (int jz = 0; jz < localmesh->LocalNz; ++jz) {
                      result[base_ind + jz] = lhs[index] * rhs[base_ind + jz];
                    });

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ wrapper for division of Field2D and Field3D
Field3D operator/(const Field2D &lhs, const Field3D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    const auto base_ind = localmesh->ind2Dto3D(index);
                    for (int jz = 0; jz < localmesh->LocalNz; ++jz) {
                      result[base_ind + jz] = lhs[index] / rhs[base_ind + jz];
                    });

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ wrapper for addition of Field2D and Field3D
Field3D operator+(const Field2D &lhs, const Field3D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    const auto base_ind = localmesh->ind2Dto3D(index);
                    for (int jz = 0; jz < localmesh->LocalNz; ++jz) {
                      result[base_ind + jz] = lhs[index] + rhs[base_ind + jz];
                    });

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ wrapper for subtraction of Field2D and Field3D
Field3D operator-(const Field2D &lhs, const Field3D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    const auto base_ind = localmesh->ind2Dto3D(index);
                    for (int jz = 0; jz < localmesh->LocalNz; ++jz) {
                      result[base_ind + jz] = lhs[index] - rhs[base_ind + jz];
                    });

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ wrapper for multiplication of Field2D and Field2D
Field2D operator*(const Field2D &lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs[index] * rhs[index];);

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field2D by multiplication with Field2D
Field2D &Field2D::operator*=(const Field2D &rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index,
                      (*this)[index] *= rhs[index];);

    checkData(*this);

  } else {
    (*this) = (*this) * rhs;
  }
  return *this;
}

// Provide the C++ wrapper for division of Field2D and Field2D
Field2D operator/(const Field2D &lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs[index] / rhs[index];);

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field2D by division with Field2D
Field2D &Field2D::operator/=(const Field2D &rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index,
                      (*this)[index] /= rhs[index];);

    checkData(*this);

  } else {
    (*this) = (*this) / rhs;
  }
  return *this;
}

// Provide the C++ wrapper for addition of Field2D and Field2D
Field2D operator+(const Field2D &lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs[index] + rhs[index];);

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field2D by addition with Field2D
Field2D &Field2D::operator+=(const Field2D &rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index,
                      (*this)[index] += rhs[index];);

    checkData(*this);

  } else {
    (*this) = (*this) + rhs;
  }
  return *this;
}

// Provide the C++ wrapper for subtraction of Field2D and Field2D
Field2D operator-(const Field2D &lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  ASSERT1(localmesh == rhs.getMesh());

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs[index] - rhs[index];);

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field2D by subtraction with Field2D
Field2D &Field2D::operator-=(const Field2D &rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    ASSERT1(fieldmesh == rhs.getMesh());

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index,
                      (*this)[index] -= rhs[index];);

    checkData(*this);

  } else {
    (*this) = (*this) - rhs;
  }
  return *this;
}

// Provide the C++ wrapper for multiplication of Field2D and BoutReal
Field2D operator*(const Field2D &lhs, const BoutReal rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs[index] * rhs;);

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field2D by multiplication with BoutReal
Field2D &Field2D::operator*=(const BoutReal rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index, (*this)[index] *= rhs;);

    checkData(*this);

  } else {
    (*this) = (*this) * rhs;
  }
  return *this;
}

// Provide the C++ wrapper for division of Field2D and BoutReal
Field2D operator/(const Field2D &lhs, const BoutReal rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs[index] / rhs;);

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field2D by division with BoutReal
Field2D &Field2D::operator/=(const BoutReal rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index, (*this)[index] /= rhs;);

    checkData(*this);

  } else {
    (*this) = (*this) / rhs;
  }
  return *this;
}

// Provide the C++ wrapper for addition of Field2D and BoutReal
Field2D operator+(const Field2D &lhs, const BoutReal rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs[index] + rhs;);

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field2D by addition with BoutReal
Field2D &Field2D::operator+=(const BoutReal rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index, (*this)[index] += rhs;);

    checkData(*this);

  } else {
    (*this) = (*this) + rhs;
  }
  return *this;
}

// Provide the C++ wrapper for subtraction of Field2D and BoutReal
Field2D operator-(const Field2D &lhs, const BoutReal rhs) {

  SCOREP0();
  Mesh *localmesh = lhs.getMesh();

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs[index] - rhs;);

  checkData(result);
  return result;
}

// Provide the C++ operator to update Field2D by subtraction with BoutReal
Field2D &Field2D::operator-=(const BoutReal rhs) {
  SCOREP0();
  // only if data is unique we update the field
  // otherwise just call the non-inplace version
  if (data.unique()) {

    checkData(*this);
    checkData(rhs);

    BLOCK_REGION_LOOP(fieldmesh->getRegion2D("RGN_ALL"), index, (*this)[index] -= rhs;);

    checkData(*this);

  } else {
    (*this) = (*this) - rhs;
  }
  return *this;
}

// Provide the C++ wrapper for multiplication of BoutReal and Field3D
Field3D operator*(const BoutReal lhs, const Field3D &rhs) {

  SCOREP0();
  Mesh *localmesh = rhs.getMesh();

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs * rhs[index];);

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ wrapper for division of BoutReal and Field3D
Field3D operator/(const BoutReal lhs, const Field3D &rhs) {

  SCOREP0();
  Mesh *localmesh = rhs.getMesh();

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs / rhs[index];);

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ wrapper for addition of BoutReal and Field3D
Field3D operator+(const BoutReal lhs, const Field3D &rhs) {

  SCOREP0();
  Mesh *localmesh = rhs.getMesh();

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs + rhs[index];);

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ wrapper for subtraction of BoutReal and Field3D
Field3D operator-(const BoutReal lhs, const Field3D &rhs) {

  SCOREP0();
  Mesh *localmesh = rhs.getMesh();

  Field3D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion3D("RGN_ALL"), index,
                    result[index] = lhs - rhs[index];);

  result.setLocation(rhs.getLocation());

  checkData(result);
  return result;
}

// Provide the C++ wrapper for multiplication of BoutReal and Field2D
Field2D operator*(const BoutReal lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = rhs.getMesh();

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs * rhs[index];);

  checkData(result);
  return result;
}

// Provide the C++ wrapper for division of BoutReal and Field2D
Field2D operator/(const BoutReal lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = rhs.getMesh();

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs / rhs[index];);

  checkData(result);
  return result;
}

// Provide the C++ wrapper for addition of BoutReal and Field2D
Field2D operator+(const BoutReal lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = rhs.getMesh();

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs + rhs[index];);

  checkData(result);
  return result;
}

// Provide the C++ wrapper for subtraction of BoutReal and Field2D
Field2D operator-(const BoutReal lhs, const Field2D &rhs) {

  SCOREP0();
  Mesh *localmesh = rhs.getMesh();

  Field2D result(localmesh);
  result.allocate();
  checkData(lhs);
  checkData(rhs);

  BLOCK_REGION_LOOP(localmesh->getRegion2D("RGN_ALL"), index,
                    result[index] = lhs - rhs[index];);

  checkData(result);
  return result;
}
