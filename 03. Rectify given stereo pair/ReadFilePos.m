function [LPoint, RPoint, LinesCount] = ReadFilePos(fileLocation)
    %  讀檔案
    %disp(fileLocation);
    fileID = fopen(fileLocation,'r');
    
    % 跳過第一航
    fgets(fileID);
    
    % 讀數字
    formatSpec = '%d';
    PosDataTxt = fscanf(fileID, formatSpec);
    
    % 打散數字
    LinesCount = 10;
    LPoint = zeros(LinesCount, 3);
    RPoint = zeros(LinesCount, 3);
    for i=1:LinesCount
        % 算 Index
        index = (i - 1) * 4 + 1;
        
        % 填入結果
        LPoint(i, 1) = PosDataTxt(index + 0);
        LPoint(i, 2) = PosDataTxt(index + 1);
        LPoint(i, 3) = 1;
        RPoint(i, 1) = PosDataTxt(index + 2);
        RPoint(i, 2) = PosDataTxt(index + 3);
        RPoint(i, 3) = 1;
    end
    fclose(fileID);
end