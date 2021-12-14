f_s = 1000;
f = [250 333 667 750 1250 1333];
[~, r] = size(f);
t = (0:1/(f_s * 100):0.025);
[~, c] = size(t);
a = 1; % amplitude
phi = 0; % phase

y_s = a * sin(2 * pi * f_s * t + phi);
y = zeros(r,c);
xints = zeros(r,50);
yints = zeros(r,50);

for i = 1:r
    y(i,:) = a * sin(2 * pi * f(i) * t + phi);

    s = figure;
    plot(t, y(i,:)); hold on
    %plot(t, y_s); hold on
    [xi, yi] = polyxpoly(t, y_s, t, y(i,:)); % find intersection points
    xints(i,:) = xi(1:50);
    yints(i,:) = yi(1:50);
    plot(xints(i,:), yints(i,:), '*r'); % plot 50 sample points
    xlabel('time (s)');
    ylabel('amplitude (V)');
    hold off

    saveas(s, sprintf('%dhz.png', f(i)));
end