spline = [
73, 73, 73, 73, 73, 73, 73, 73, 72, 72, 72, 71,
71, 71, 70, 70, 69, 69, 68, 67, 67, 66, 65, 65,
64, 63, 62, 61, 60, 60, 59, 58, 57, 56, 55, 54,
52, 51, 50, 49, 48, 47, 45, 44, 43, 42, 40, 39,
38, 36, 35, 33, 32, 30, 29, 28, 26, 25, 23, 22,
20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,
2 ]

print <<JSON
{
  "type": "FeatureCollection",
  "features": [
JSON

(-72..72).each {|j|
  ncols = (spline[j.abs] - 1) * 4
  lat = j * 90.0 / 72.0
  (0..(ncols-1)).each {|i|
    puts ',' unless j == -72 and i == 0
    lon = i * 360.0 / ncols
    print '{"type": "Feature", "geometry": {"type": "Point", "coordinates":'
    printf '[%3.4g, %3.4g]', lon, lat
    print '}}'
  }
}

print <<JSON
  ]
}
JSON
