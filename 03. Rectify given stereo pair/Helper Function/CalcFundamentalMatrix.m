function invMatrix = CalcFundamentalMatrix(NPoint1, NPoint2, LinesCount)
    FundamentalMatrix = zeros(LinesCount, 8)
    AnsVector = zeros(8, 1)
    
    % 2 => 1
    for rows=1:LinesCount
        FundamentalMatrix(rows, 1) = NPoint2(rows, 1) * NPoint1(rows, 1)
        FundamentalMatrix(rows, 2) = NPoint2(rows, 1) * NPoint1(rows, 2)
        FundamentalMatrix(rows, 3) = NPoint2(rows, 1) * 1
        FundamentalMatrix(rows, 4) = NPoint2(rows, 2) * NPoint1(rows, 1)
        FundamentalMatrix(rows, 5) = NPoint2(rows, 2) * NPoint1(rows, 2)
        FundamentalMatrix(rows, 6) = NPoint2(rows, 2) * 1
        FundamentalMatrix(rows, 7) = NPoint1(rows, 1)
        FundamentalMatrix(rows, 8) = NPoint1(rows, 2)
    end
    
    % -1
    for i=1:8
        AnsVector(i, 1) = -1
    end
    
    invMatrix = transpose(FundamentalMatrix) * FundamentalMatrix * AnsVector
end