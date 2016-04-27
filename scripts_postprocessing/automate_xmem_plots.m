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
% This script plots relevant results from X-Mem experiments across different machine configurations.

%% Plot xmem throughput results and save the figures
for i=1:size(nice_xmem_throughput_results,3)
    experiment_types{i} = 'throughput';
    y_labels{i} = 'Average Memory Throughput (MB/s)';
end
plot_nice_xmem_results(nice_xmem_throughput_results, nice_xmem_throughput_column_headers, xmem_experiment_names, experiment_types, y_labels, xmem_processed_data_dir);

%% Plot xmem latency results and save the figures
for i=1:size(nice_xmem_latency_results,3)
    experiment_types{i} = 'latency';
    y_labels{i} = 'Average Memory Latency (ns/access)';
end
plot_nice_xmem_results(nice_xmem_latency_results, nice_xmem_latency_column_headers, xmem_experiment_names, experiment_types, y_labels, xmem_processed_data_dir);
