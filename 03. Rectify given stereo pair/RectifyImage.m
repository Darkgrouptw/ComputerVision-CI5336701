% 初始化設定
addpath('./Helper Function/');
clc;clear;

% 初始化設定
LImg = imread('./Data/L.jpg');
RImg = imread('./Data/R.jpg');
[rows, cols, channels] = size(LImg);



% 拿資料並 Normalize
[LPoints, RPoints, LinesCount] = ReadFilePos('./Data/FeaturesPoints.txt');
NLPoints = NormalizePoint(LPoints, cols, rows,  LinesCount);
NRPoints = NormalizePoint(RPoints, cols, rows,  LinesCount);

figure;
showMatchedFeatures(LImg, RImg, LPoints(:, 1:2), RPoints(:, 1:2),'montage','PlotOptions',{'ro','go','y-'});
title('Putative point matches');

CalcFundamentalMatrix(LPoints, RPoints, LinesCount);
