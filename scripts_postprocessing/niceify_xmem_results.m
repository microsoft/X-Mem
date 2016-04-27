function [nice_xmem_results, column_headers] = niceify_xmem_results(xmem_results, cpu_nodes, mem_nodes, access_patterns, rw_patterns, chunks, strides, working_sets)
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
%
% Parses the xmem_results cell array, which is basically a mirror of the
% original CSV inputs, into a nice matrix of doubles that are suitable for
% further analysis.
%
% Arguments:
%   xmem_results -- MxNxK Cell Array of Strings. M is the number of
%       tests+1, N is the number of columns in the original CSVs, and K is the
%       number of xmem configurations.
%   cpu_nodes -- Zx1 Cell Array of Strings. Z is the number of NUMA nodes
%       to look for in xmem_results.
%   mem_nodes -- Zx1 Cell Array of Strings. Z is the number of NUMA nodes
%       to look for in xmem_results.
%   access_patterns -- Ax1 Cell Array of Strings. A is the number of access
%       patterns to look for in xmem_results.
%   rw_patterns -- Px1 Cell Array of Strings. P is the number of read/write patterns to
%       look for in xmem_results.
%   chunks -- Cx1 Cell Array of Strings. C is the number of chunks to look
%       for in xmem_results.
%   strides -- Sx1 Cell Array of Strings. S is the number of strides to
%       look for in xmem_results.
%   working_sets -- Wx1 Vector. W is the number of working set sizes.
%
% Returns:
%   nice_xmem_results -- (M-1)xBxK Matrix. B is the total number of
%       cpu_node/mem_node/access pattern/rw pattern/chunk/stride combinations.
%   column_headers -- 1x(B+2) Cell Array of Strings.

% Niceify each xmem experiment
% Yes, I know this is the world's worst nested for loop. But it works, and I don't care about performance here.
for i=1:size(xmem_results,3)  
   % Create index and working set size columns in the nice xmem results
   output_row = 1;
   for w=1:size(working_sets)
       nice_xmem_results(output_row,1,i) = output_row; % Index
       nice_xmem_results(output_row,2,i) = working_sets(w);
       output_row = output_row+1;
   end
      
   % Build out columns with results for different test types (cpu & mem nodes, access patterns, rw patterns, chunks, and strides)
   col = 3;
   for cpu_node=1:size(cpu_nodes,1) % For each CPU node
       for mem_node=1:size(mem_nodes,1) % For each mem node
           for a=1:size(access_patterns,1) % For each access pattern
               for p=1:size(rw_patterns,1) % For each RW pattern
                   for c=1:size(chunks,1) % For each chunk
                       for s=1:size(strides,1) % For each stride
                           % Extract relevant data
                           for r=2:size(xmem_results,1) 
                                match_cpu_node = strcmp(xmem_results{r,7,i}, cpu_nodes{cpu_node});
                                match_mem_node = strcmp(xmem_results{r,6,i}, mem_nodes{mem_node});
                                match_access_pattern = strcmp(xmem_results{r,8,i}, access_patterns{a});
                                match_rw_pattern = strcmp(xmem_results{r,9,i}, rw_patterns{p});
                                match_chunk = strcmp(xmem_results{r,10,i}, chunks{c});
                                match_stride = strcmp(xmem_results{r,11,i}, strides{s});
                                if match_cpu_node == 1 && match_mem_node == 1 && match_access_pattern && match_rw_pattern == 1 && match_chunk && match_stride == 1
                                    w_tmp = str2double(xmem_results{r,3,i}); % Extract working set size
                                    val_tmp = str2double(xmem_results{r,12,i}); % Extract the result value
                                    %avgpower_socket0_tmp = str2double(xmem_results{r,12,i}); % Extract socket 0 average power
                                    %peakpower_socket0_tmp = str2double(xmem_results{r,13,i}); % Extract socket 0 peak power
                                    %avgpower_socket1_tmp = str2double(xmem_results{r,14,i}); % Extract socket 1 average power
                                    %peakpower_socket1_tmp = str2double(xmem_results{r,15,i}); % Extract socket 1 peak power

                                    % Insert result into output matrix for this matched working set size
                                    for output_row = 1:size(nice_xmem_results,1)
                                        if nice_xmem_results(output_row,2,i) == w_tmp % Match
                                            nice_xmem_results(output_row,col,i) = val_tmp;
                                            %nice_xmem_results(output_row,col,i) = avgpower_socket0_tmp;
                                            %nice_xmem_results(output_row,col,i) = peakpower_socket0_tmp;
                                            %nice_xmem_results(output_row,col,i) = avgpower_socket1_tmp;
                                            %nice_xmem_results(output_row,col,i) = peakpower_socket1_tmp;
                                            break;
                                        end
                                    end
                                end
                           end
                           col = col+1;
                       end
                   end
               end
           end
       end
   end
end

% Generate column headers
% Yes, I know this is the world's worst nested for loop. But it works, and I don't care about performance here.
column_headers{1} = 'Index';
column_headers{2} = 'Working Set Size (KB)';
col = 3;
for cpu_node=1:size(cpu_nodes,1) % For each CPU node
    for mem_node=1:size(mem_nodes,1) % For each mem node
        for a=1:size(access_patterns,1) % For each access pattern
            for p=1:size(rw_patterns,1) % For each RW pattern
               for c=1:size(chunks,1) % For each chunk
                   for s=1:size(strides,1) % For each stride
                       column_headers{col} = ['CPU Node: ' cpu_nodes{cpu_node} ' Mem Node: ' mem_nodes{mem_node} ' Access Pattern: ' access_patterns{a} ' R/W Pattern: ' rw_patterns{p} ' Chunk: ' chunks{c} ' Stride: ' strides{s}];
                       col = col+1;
                   end
               end
            end
        end
    end
end


end

