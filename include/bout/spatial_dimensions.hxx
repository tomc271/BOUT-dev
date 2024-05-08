#ifndef BOUT_SPATIAL_DIMENSIONS_HXX
#define BOUT_SPATIAL_DIMENSIONS_HXX

#if BOUT_USE_METRIC_3D
    #include "field3d.hxx"
#else
    #include "field2d.hxx"
#endif

class SpatialDimensions {

  public:
#if BOUT_USE_METRIC_3D
    using FieldMetric = Field3D;
#else
    using FieldMetric = Field2D;
#endif

};

#endif //BOUT_SPATIAL_DIMENSIONS_HXX
