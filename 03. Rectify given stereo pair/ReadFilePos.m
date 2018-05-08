function [LPoint, RPoint, LinesCount] = ReadFilePos(fileLocation)
    %  Ū�ɮ�
    %disp(fileLocation);
    fileID = fopen(fileLocation,'r');
    
    % ���L�Ĥ@��
    fgets(fileID);
    
    % Ū�Ʀr
    formatSpec = '%d';
    PosDataTxt = fscanf(fileID, formatSpec);
    
    % �����Ʀr
    LinesCount = 10;
    LPoint = zeros(LinesCount, 3);
    RPoint = zeros(LinesCount, 3);
    for i=1:LinesCount
        % �� Index
        index = (i - 1) * 4 + 1;
        
        % ��J���G
        LPoint(i, 1) = PosDataTxt(index + 0);
        LPoint(i, 2) = PosDataTxt(index + 1);
        LPoint(i, 3) = 1;
        RPoint(i, 1) = PosDataTxt(index + 2);
        RPoint(i, 2) = PosDataTxt(index + 3);
        RPoint(i, 3) = 1;
    end
    fclose(fileID);
end