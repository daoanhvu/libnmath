tx = ty = linspace (-0.5, 0.5, 15);
[xx, yy] = meshgrid (tx, ty);

mesh (tx, ty, xx .* sin(yy));
xlabel ("tx");
ylabel ("ty");
zlabel ("tz");
title ("f(x,y) = x * sin(y)");
hold on

nv1 = quiver3(-0.5, -0.5, 0.239713, 0.401987, 0.367916, 0.838477)
hold on
nv2 = quiver3(-0.5, 0, 0, -0, 0.447214, 0.894427)
hold on
nv3 = quiver3(-0.5, 0.5, -0.239713, -0.401987, 0.367916, 0.838477)
hold on
nv4 = quiver3(0, -0.5, 0, 0.43231, -0, 0.901725)
hold on
nv5 = quiver3(0, 0, 0, 0, 0, 1)
hold on
nv6 = quiver3(0, 0.5, 0,-0.43231, -0, 0.901725)
hold on
nv7 = quiver3(0.5, -0.5, -0.239713, 0.401987, -0.367916, 0.838477)
hold on
nv8 = quiver3(0.5, 0, 0, -0, -0.447214, 0.894427)
hold on
nv9 = quiver3(0.5, 0.5, 0.239713, -0.401987, -0.367916, 0.838477)
hold on

set(nv1, 'Color', 'blue');
set(nv2, 'Color', 'blue');
set(nv3, 'Color', 'blue');
set(nv4, 'Color', 'blue');
set(nv5, 'Color', 'blue');
set(nv6, 'Color', 'blue');
set(nv7, 'Color', 'blue');
set(nv8, 'Color', 'blue');
set(nv9, 'Color', 'blue');

set(nv1, 'LineWidth', 1);
set(nv2, 'LineWidth', 1);
set(nv3, 'LineWidth', 1);
set(nv4, 'LineWidth', 1);
set(nv5, 'LineWidth', 1);
set(nv6, 'LineWidth', 1);
set(nv7, 'LineWidth', 1);
set(nv8, 'LineWidth', 1);
set(nv9, 'LineWidth', 1);

axis equal

textx = text(-0.401987+0.5, -0.367916+0.5, 0.838477+0.239713, 'n9')
set(textx, 'Color', 'blue')