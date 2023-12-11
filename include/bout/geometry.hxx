/**************************************************************************
 * Describes coordinate geometry
 *
 * ChangeLog
 * =========
 * 
 * 2021-11-23 Tom Chapman <tpc522@york.ac.uk>
 *    * Extracted from Coordinates
 *
 * 
 **************************************************************************
 * Copyright 2014 B.D.Dudson
 *
 * Contact: Ben Dudson, bd512@york.ac.uk
 * 
 * This file is part of BOUT++.
 *
 * BOUT++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BOUT++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOUT++.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************/

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include "christoffel_symbols.hxx"
#include "differential_operators.hxx"
#include "metricTensor.hxx"

using FieldMetric = MetricTensor::FieldMetric;

/*!
 * Represents the geometry a coordinate system, and associated operators
 *
 * This is a container for a collection of metric tensor components
 */
class Geometry {

public:
  Geometry(FieldMetric J, FieldMetric Bxy, const FieldMetric& g11, const FieldMetric& g22,
           const FieldMetric& g33, const FieldMetric& g12, const FieldMetric& g13,
           const FieldMetric& g23, const FieldMetric& g_11, const FieldMetric& g_22,
           const FieldMetric& g_33, const FieldMetric& g_12, const FieldMetric& g_13,
           const FieldMetric& g_23, DifferentialOperators* differential_operators);

  Geometry(Mesh* mesh, DifferentialOperators* differential_operators);

  /// Covariant metric tensor
  const FieldMetric& g_11() const;
  const FieldMetric& g_22() const;
  const FieldMetric& g_33() const;
  const FieldMetric& g_12() const;
  const FieldMetric& g_13() const;
  const FieldMetric& g_23() const;

  /// Contravariant metric tensor (g^{ij})
  const FieldMetric& g11() const;
  const FieldMetric& g22() const;
  const FieldMetric& g33() const;
  const FieldMetric& g12() const;
  const FieldMetric& g13() const;
  const FieldMetric& g23() const;

  const MetricTensor& getContravariantMetricTensor() const;
  const MetricTensor& getCovariantMetricTensor() const;

  /// Christoffel symbol of the second kind (connection coefficients)
  const FieldMetric& G1_11() const;
  const FieldMetric& G1_22() const;
  const FieldMetric& G1_33() const;
  const FieldMetric& G1_12() const;
  const FieldMetric& G1_13() const;
  const FieldMetric& G1_23() const;

  const FieldMetric& G2_11() const;
  const FieldMetric& G2_22() const;
  const FieldMetric& G2_33() const;
  const FieldMetric& G2_12() const;
  const FieldMetric& G2_13() const;
  const FieldMetric& G2_23() const;

  const FieldMetric& G3_11() const;
  const FieldMetric& G3_22() const;
  const FieldMetric& G3_33() const;
  const FieldMetric& G3_12() const;
  const FieldMetric& G3_13() const;
  const FieldMetric& G3_23() const;

  const FieldMetric& G1() const;
  const FieldMetric& G2() const;
  const FieldMetric& G3() const;

  void setG1(FieldMetric G1);
  void setG2(FieldMetric G2);
  void setG3(FieldMetric G3);

  ///< Coordinate system Jacobian, so volume of cell is J*dx*dy*dz
  const FieldMetric& J() const;

  ///< Magnitude of B = nabla z times nabla x
  const FieldMetric& Bxy() const;

  void setContravariantMetricTensor(const MetricTensor& metric_tensor,
                                    const std::string& region = "RGN_ALL");

  void setCovariantMetricTensor(const MetricTensor& metric_tensor,
                                const std::string& region = "RGN_ALL");

  void setJ(FieldMetric J);
  void setJ(BoutReal value, int x, int y);

  void setBxy(FieldMetric Bxy);

  /// Calculate Christoffel symbol terms
  void CalculateChristoffelSymbols(const FieldMetric& dx, const FieldMetric& dy);

  // check that covariant tensors are positive (if expected) and finite (always)
  void checkCovariant(int ystart);

  // check that contravariant tensors are positive (if expected) and finite (always)
  void checkContravariant(int ystart);

  FieldMetric recalculateJacobian();
  FieldMetric recalculateBxy();

  void applyToContravariantMetricTensor(
      const std::function<const FieldMetric(const FieldMetric)>& function);

  void applyToCovariantMetricTensor(
      const std::function<const FieldMetric(const FieldMetric)>& function);

  void
  applyToChristoffelSymbols(const std::function<const FieldMetric(const FieldMetric)>& function);

private:
  /// Christoffel symbol of the second kind (connection coefficients)
  ChristoffelSymbols christoffel_symbols;

  MetricTensor contravariantMetricTensor;
  MetricTensor covariantMetricTensor;

  FieldMetric this_J;
  FieldMetric this_Bxy; ///< Magnitude of B = nabla z times nabla x

  DifferentialOperators* differential_operators;
};

#endif // __GEOMETRY_H__
