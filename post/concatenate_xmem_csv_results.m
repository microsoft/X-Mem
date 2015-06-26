function [result] = concatenate_xmem_csv_results(raw_results_directory, processed_csv_filename)
% The MIT License (MIT)
% 
% Copyright (c) 2014 Microsoft
% 
% Permission is hereby granted, free of charge, to any person obtaining a copy
% of this software and associated documentation files (the "Software"), to deal
% in the Software without restriction, including without limitation the rights
% to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
% copies of the Software, and to permit persons to whom the Software is
% furnished to do so, subject to the following conditions:
% 
% The above copyright notice and this permission notice shall be included in all
% copies or substantial portions of the Software.
% 
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
% IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
% OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
% SOFTWARE.
%
% Author: Mark Gottscho <mgottscho@ucla.edu>
%
% Concatenates all xmem CSV files from a result directory into a single
% CSV to make further analysis easier.
%
% Arguments:
%   raw_results_directory -- A string representing a path to the location
%       of CSV files. Note that all CSV files should have equal number of
%       columns, and it is assumed the first row in each file is a header row.
%   processed_csv_filename -- A string representing a path to the file to
%       create containing the concatenated results.
%
% Returns:
%   result -- xmem results in a single cell array representing the concatenated set
%       of data.

xmem_result_files = dir([raw_results_directory filesep '*.csv']);

row = 1;
first_file = 1;

% Get dimensions of the first CSV to set size of results
test_result = read_mixed_csv([raw_results_directory filesep xmem_result_files(1).name],',');
result = cell(size(test_result,1),size(test_result,2));
for file = xmem_result_files'
    single_result = read_mixed_csv([raw_results_directory filesep file.name],',');
    if first_file == 1
        result_row = 1;
        first_file = 0;
    else
        result_row = 2;
    end

    while result_row <= size(single_result,1)
        result(row,:) = single_result(result_row,:);
        row = row+1;
        result_row = result_row+1;
    end
end

% Write to the concatenated CSV file
fid = fopen(processed_csv_filename, 'w');
for r = 1:size(result,1)
   for c = 1:size(result,2)
       fprintf(fid, '%s,', result{r,c});
   end
   fprintf(fid, '\n');
end
fclose(fid);
