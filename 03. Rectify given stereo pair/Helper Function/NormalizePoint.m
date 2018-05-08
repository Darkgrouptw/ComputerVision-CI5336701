function Points = NormalizePoint(Points, Width, Height, PointsCount)
    for i=1:PointsCount
        Points(i, 1) = Points(i, 1) / Width;
        Points(i, 2) = Points(i, 2) / Height;
    end
end