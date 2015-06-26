function [] = plot_nice_xmem_results(nice_xmem_results, column_headers, experiment_names, experiment_types, y_labels, output_dir)
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
% Plots X-Mem results in a very nice format so it is easy to analyze.
% Saves figures to disk in various useful formats as well.
%
% Arguments:
%   nice_xmem_results -- MxNxK Matrix. M is the number of data points in
%       each line+1, N is the number of lines on a plot+2, and K is the number of
%       figures to make.
%   column_headers -- 1xN Cell Array of Strings. 
%   experiment_names -- 1xK Cell Array of Strings.
%   experiment_types -- 1xK Cell Array of Strings.
%   y_labels -- 1xK Cell Array of Strings.
%   output_dir -- String. Path to base output directory to write figures.

% Loop over all configurations
for i=1:size(nice_xmem_results,3)
    % Make one figure for each configuration
    figure;
    hold on;
    
    % Make X and Y plot matrices
    X = nice_xmem_results(:,1,i);
    for j=2:size(nice_xmem_results,2)-2
        X(:,j) = nice_xmem_results(:,1,i);
    end
    
    Y = nice_xmem_results(:,3:size(nice_xmem_results,2),i);
    
    mycolors = copper(size(X,2));
    for j=1:size(X,2)
        plot(X(:,j),Y(:,j),'-o','LineWidth',1.5,'Color',mycolors(j,:));
    end

    % X axis formatting
    xlabel('Working Set Size (KB)');
    % Convert X tick labels to strings
    for j=1:size(nice_xmem_results,1)
        if mod(j,2) == 1 % Odd
            x_labels{j} = num2str(nice_xmem_results(j,2,1));
        else
            x_labels{j} = '';
        end
    end
    set(gca, 'XTick', nice_xmem_results(1:1:size(nice_xmem_results,1),1,1)); % Every other tick
    set(gca, 'XTickLabel', x_labels);
    
    % Y axis formatting
    ylabel(y_labels(i));
    ylim([0 max(max(Y))*1.2]); % Make sure Y axis is 0-based
    % Convert Y tick labels to strings
    yticks = get(gca, 'ytick');
    for j=1:size(yticks,2)
        y_tick_labels{j} = num2str(yticks(j));
    end
    set(gca, 'YTickLabel', y_tick_labels);
    
    % Title and legend
    title(['xmem ' experiment_types{i} ' experiment: ' strrep(experiment_names{i},'_','\_')]);
    legend(column_headers{1,3:size(column_headers,2)});
   
    % Save figure as .fig and as .pdf
    filename_dir = [output_dir '/' experiment_names{i} '/figures'];
    mkdir(filename_dir);
    filename_prefix = [filename_dir '/' experiment_names{i} '_' experiment_types{i}];
    savefig(filename_prefix);
    print('-dpdf', filename_prefix);
    print('-depsc2', filename_prefix);
    print('-dpng', filename_prefix);
    print('-djpeg90', filename_prefix);
    
    hold off;
    close;
end
end
