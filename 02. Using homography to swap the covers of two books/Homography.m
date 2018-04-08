function Homography()
    %  讀檔案
    fileID = fopen('pos.txt','r');
    formatSpec = '%d';
    PosDataTxt = fscanf(fileID,formatSpec);
    PosData = PosDataTXT_2_Pos(PosDataTxt);
    
    % 設定矩陣
    [Matrix1To2, Basis1] = SetInhomographyMatrix(PosData(1:4,:,:), PosData(5:8,:,:));
    Matrix1To2 = pinv(transpose(Matrix1To2) * Matrix1To2) * transpose(Matrix1To2) * Basis1;
    Matrix1To2 = [transpose(Matrix1To2(1:3)); transpose(Matrix1To2(4:6)); transpose(Matrix1To2(7:8)), 1];
    
    Matrix2To1 = pinv(Matrix1To2);
    
    % 圖片
    img = imread('img.jpg');
    %img = imresize(img, 0.25);
    [rows, cols, channels] = size(img);
    imgClone = img;
    
    % 取出要的部分
    for i = 1: rows
        for j = 1: cols
            dot = [i, j, 1];
            if CheckIsInsideArea(PosData(1:4,:,:), dot(1:2))
                m2Dot = Matrix1To2 * transpose(dot);
                [x ,y] = DivideByZ(m2Dot);
                imgClone(i, j,:) = img(x,y,:);
            end
            
            if CheckIsInsideArea(PosData(5:8,:,:), dot(1:2))
                m2Dot = Matrix2To1 * transpose(dot);
                [x ,y] = DivideByZ(m2Dot);
                %disp([x, y]);
                
                imgClone(i, j,:) = img(x,y,:);
            end
        end
    end
    
    %imshow(imgClone);
    imwrite(imgClone, './M10515102.jpg')
end


% 轉換成三為座標
function pos = PosDataTXT_2_Pos(vector)
    [len, v] = size(vector);
    
    pos = zeros(len/2, 3);
    for i = 1:len /2
        pos(i,:) = [vector(i * 2 - 1), vector(i * 2), 1];
        %pos(i,:) = [vector(i * 2 - 1) / 4, vector(i * 2) / 4, 1];
    end
end

% 給四個點 & 一個點，判斷一個點是否在四邊形裡面
function bool = CheckIsInsideArea(pos, dot)
    orgArea = CalcThreePointArea(pos(1:3,:,1)) + CalcThreePointArea(pos([1,3,4],:,1));
    
    % 四個陣列
    list1 = zeros(3, 2);
    list1(1:2, :) = pos(1:2, 1:2);
    list1(3,:) = dot;
    list2 = zeros(3, 2);
    list2(1:2, :) = pos(2:3, 1:2);
    list2(3,:) = dot;
    list3 = zeros(3, 2);
    list3(1:2, :) = pos(3:4, 1:2);
    list3(3,:) = dot;
    list4 = zeros(3, 2);
    list4(1:2, :) = pos([1, 4], 1:2);
    list4(3,:) = dot;
    dotArea = CalcThreePointArea(list1) + CalcThreePointArea(list2) +  CalcThreePointArea(list3) + CalcThreePointArea(list4);
    
    if abs(orgArea - dotArea) < 0.01
        bool = true;
    else
        bool = false;
    end
end

% 給三個點算距離
function area = CalcThreePointArea(pos)
    area = abs(pos(1,1) * pos(2,2) + pos(2,1) * pos(3,2) + pos(3,1) * pos(1,2) - pos(2,1) * pos(1,2) - pos(3,1) * pos(2,2) - pos(1,1) * pos(3,2)) / 2;
end

% 設定矩陣
function [result, basis] = SetInhomographyMatrix(pos1, pos2)
    result = zeros(8, 8);
    basis = zeros(8, 1);
    for i = 1: 4
        % 設定前面的矩陣
        result(2 * i - 1, 4:6) = -pos1(i, :);
        result(2 * i, 1:3) = pos1(i, :);
        
        % 設定後面的矩陣
        result(2 * i - 1, 7:8) = pos1(i, 1:2) * pos2(i, 2);
        result(2 * i, 7:8) = -pos1(i, 1:2) * pos2(i, 1);
        
        basis(2 * i - 1, 1) = -pos2(i , 2);
        basis(2 * i, 1) = pos2(i, 1);
    end
end

% 重新轉成 2D
function [x ,y] = DivideByZ(pos)
    z = pos(3);
    x = int32(pos(1) / z);
    y = int32(pos(2) / z);
end