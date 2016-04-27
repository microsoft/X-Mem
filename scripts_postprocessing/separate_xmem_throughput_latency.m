function [xmem_throughput_results, xmem_latency_results] = separate_xmem_throughput_latency(xmem_results)
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
% Splits a 3D cell array of xmem results into two 3D cell arrays by
% throughput and latency.
%
% Arguments:
%   xmem_results -- MxNxK cell array. xmem results in a single 3D cell array representing
%   the concatenated sets of data. M is the max number of tests+1 for each experiment. N is the
%   number of columns in the experiment. K is the number of experiments.
%
% Returns:
%   xmem_throughput_results -- PxNxK cell array. Same contents as
%       xmem_results, but with reduced number of rows such that only
%       throughput tests are included.
%
%   xmem_latency_results -- QxNxK cell array. Same contents as
%       xmem_results, but with reduced number of rows such that only
%       latency tests are included.
%
%   P+Q-1 == M.

% Init
xmem_throughput_results = cell(1,size(xmem_results,2),size(xmem_results,3)); 
xmem_latency_results = cell(1,size(xmem_results,2),size(xmem_results,3)); 

% Copy headers
xmem_throughput_results(1,:,:) = xmem_results(1,:,:);
xmem_latency_results(1,:,:) = xmem_results(1,:,:);

% Separate throughput and latency results into the two 3D cell arrays
for i=1:size(xmem_results,3) % For each experiment
    t_r = 2;
    l_r = 2;
    for r=2:size(xmem_results,1) % Skip header row  
        % If throughput
        locs = strfind(xmem_results{r,1,i}, 'Throughput');
        if size(locs,2) > 0 % Found throughput
            xmem_throughput_results(t_r,:,i) = xmem_results(r,:,i);        
            t_r = t_r+1;
        end

        % If latency
        locs = strfind(xmem_results{r,1,i}, 'Latency');
        if size(locs,2) > 0 % Found latency
            xmem_latency_results(l_r,:,i) = xmem_results(r,:,i);    
            l_r = l_r+1;
        end
    end
end

end

