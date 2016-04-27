function [full_file_path] = post_process_xmem(machine_name, machine_config, top_data_dir)
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
% This function post-processes X-Mem results from experiments on a machine.
% master-automator.cmd and run-experiment.cmd scripts.
%
% Arguments:
%   machine_name -- name of the machine that ran the experiment
%   machine_config -- name of directory containing a machine configuration (e.g., OS/HW setup) to post-process.
%   top_data_dir -- full path to the top-level data directory with X-Mem results
% 
% Returns:
%   full_file_path -- String of the full path to the output .mat file
%       containing the post-processed X-Mem results.

%% Input configuration
%%%%%%%%%%% CHANGE THESE AS NEEDED %%%%%%%%%%%%
raw_data_dir = [top_data_dir filesep 'raw' filesep machine_name];
processed_data_dir = [top_data_dir filesep 'processed' filesep machine_name];
xmem_raw_data_dir = [raw_data_dir filesep 'xmem' filesep machine_config];
xmem_processed_data_dir = [processed_data_dir filesep 'xmem' filesep machine_config];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Spit out the directory paths to use
display(['Top data directory:               ' top_data_dir]);
display(['Raw X-Mem data directory:         ' xmem_raw_data_dir]);
display(['--------']);
display(['^^^^^ NOTE: This directory should ONLY contain subdirectories with X-Mem results for each machine configuration. This also means no regular files or hidden files. Sorry. In the future hopefully this will be made more robust :)']);
display(['--------']);
display(['Processed X-Mem data directory:   ' xmem_processed_data_dir]);

% Make required directories if they don't already exist
mkdir(processed_data_dir);
mkdir(xmem_processed_data_dir);

%% Merge X-Mem results into single CSV file

% First find the appropriate maximum dimensions for concatenated xmem csv
% results and generate all the concatenated CSVs for all xmem experiments
max_dim_1 = 1;
max_dim_2 = 1;

xmem_experiment_dirs = dir(xmem_raw_data_dir);
i = 1;
for xmem_experiment = xmem_experiment_dirs'
    if strcmp(xmem_experiment.name,'.') == 1 || strcmp(xmem_experiment.name,'..') == 1 || strcmp(xmem_experiment.name,'.DS_Store') == 1 % Exclude curr directory and parent directory folders, hidden files. TODO: make this more robust against other files, especially hidden ones on various platforms.
        continue;
    end
    filename_dir = [xmem_processed_data_dir filesep xmem_experiment.name];
    mkdir(filename_dir);

    xmem_result = concatenate_xmem_csv_results([xmem_raw_data_dir filesep xmem_experiment.name], [filename_dir filesep xmem_experiment.name '.csv']);

    if size(xmem_result,1) > max_dim_1
        max_dim_1 = size(xmem_result,1);
    end
    
    if size(xmem_result,2) > max_dim_2
        max_dim_2 = size(xmem_result,2);
    end

    i = i+1;
end

%% Store merged X-Mem data into single data structure

% Init
xmem_results = cell(max_dim_1, max_dim_2,1); 
    
% Now re-generate all the concatenated CSVs for all xmem experiments, but
% this time store them in xmem_results
xmem_experiment_dirs = dir(xmem_raw_data_dir);
i = 1;
for xmem_experiment = xmem_experiment_dirs'
    if strcmp(xmem_experiment.name,'.') == 1 || strcmp(xmem_experiment.name,'..') == 1 || strcmp(xmem_experiment.name,'.DS_Store') == 1 % Exclude curr directory and parent directory folders, hidden files. TODO: make this more robust against other files, especially hidden ones on various platforms.
        continue;
    end
    filename_dir = [xmem_processed_data_dir filesep xmem_experiment.name];
    mkdir(filename_dir);
    xmem_result = concatenate_xmem_csv_results([xmem_raw_data_dir filesep xmem_experiment.name], [filename_dir filesep xmem_experiment.name '.csv']);
    
    xmem_results(1:size(xmem_result,1),1:size(xmem_result,2),i) = xmem_result;
    xmem_experiment_names{i} = xmem_experiment.name;
    i = i+1;
end

%% Separate xmem results into throughput and latency cell matrices so we can plot and analyze them easier.
[xmem_throughput_results, xmem_latency_results] = separate_xmem_throughput_latency(xmem_results);

%%%%%%%%%%% CHANGE THIS AS NEEDED %%%%%%%%%%%%
% TODO: make this totally automatic
% Build working sets vector for xmem
working_set_sizes(1,1) = 4;         % 4 KiB
working_set_sizes(2,1) = 8;         % 8 KiB
working_set_sizes(3,1) = 16;        % 16 KiB
working_set_sizes(4,1) = 32;        % 32 KiB
working_set_sizes(5,1) = 64;        % 64 KiB
working_set_sizes(6,1) = 128;       % 128 KiB
working_set_sizes(7,1) = 256;       % 256 KiB
working_set_sizes(8,1) = 512;       % 512 KiB
working_set_sizes(9,1) = 1024;      % 1 MiB
working_set_sizes(10,1) = 2048;      % 2 MiB
working_set_sizes(11,1) = 4096;      % 4 MiB
working_set_sizes(12,1) = 8192;      % 8 MiB
working_set_sizes(13,1) = 16384;     % 16 MiB   
working_set_sizes(14,1) = 32768;     % 32 MiB
working_set_sizes(15,1) = 65536;     % 64 MiB
working_set_sizes(16,1) = 131072;    % 128 MiB
working_set_sizes(17,1) = 262144;    % 256 MiB
working_set_sizes(18,1) = 524288;    % 512 MiB
working_set_sizes(19,1) = 1048576;   % 1 GiB
% w = 4; % 4 KB to 1 GB
% i = 1;
% while w <= 1048576
%    working_set_sizes(i,1) = w; 
%    w = w*2;
%    i = i+1;
% end

% Post-process X-Mem throughput results a bit more
% Build nodes, patterns and strides cell vectors
% TODO: make this totally automatic based on input data
cpu_nodes = {'0'; '1'};
mem_nodes = {'0'; '1'};
access_patterns = {'SEQUENTIAL'; 'RANDOM'};
rw_patterns = {'READ'; 'WRITE'};
chunks = {'32'; '64'; '128'; '256'};
strides = {'1';'-1';'2';'-2';'4';'-4';'8';'-8';'16';'-16'};
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

[nice_xmem_throughput_results, nice_xmem_throughput_column_headers] = niceify_xmem_results(xmem_throughput_results, cpu_nodes, mem_nodes, access_patterns, rw_patterns, chunks, strides, working_set_sizes);

% Post-process xmem latency results a bit more
% Build nodes, patterns and strides cell vectors
[nice_xmem_latency_results, nice_xmem_latency_column_headers] = niceify_xmem_results(xmem_latency_results, cpu_nodes, mem_nodes, access_patterns, rw_patterns, chunks, strides, working_set_sizes);

%% Clean up at this point
clear xmem_experiment;
clear xmem_result;
clear w;
clear patterns;
clear strides;
clear i;
clear ans;
clear working_set_sizes;
clear max_dim_1;
clear max_dim_2;

%% DONE!

% Save this workspace to a file
full_file_path = [xmem_processed_data_dir filesep 'xmem_post_processed.mat'];
save(full_file_path);

display(['Saved the post-processed xmem results to ' full_file_path]);
display('You can load it now to continue working.');
