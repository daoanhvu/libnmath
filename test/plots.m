data = [-0.5, -0.5, 0.239713, 0.401987, 0.367916, 0.838477, 
        -0.5, 0, -0, -0, 0.447214, 0.894427, 
        -0.5, 0.5, -0.239713, -0.401987, 0.367916, 0.838477, 
        0, -0.5, -0, 0.43231, -0, 0.901725, 
        0, 0, 0, -0, -0, 1, 
        0, 0.5, 0, -0.43231, -0, 0.901725, 
        0.5, -0.5, -0.239713, 0.401987, -0.367916, 0.838477, 
        0.5, 0, 0, -0, -0.447214, 0.894427, 
        0.5, 0.5, 0.239713, -0.401987, -0.367916, 0.838477]

n1 = [0.401987, 0.367916, 0.838477]
n2 = [0, 0.447214, 0.894427]
n3 = [-0.401987, 0.367916, 0.838477]
n4 = [0.43231, -0, 0.901725]
n5 = [0, 0, 1]
n6 = [-0.43231, -0, 0.901725]
n7 = [0.401987, -0.367916, 0.838477]
n8 = [-0, -0.447214, 0.894427]
n9 = [-0.401987, -0.367916, 0.838477]

nv1 = quiver3(-0.5, -0.5, 0.239713, 0.401987-0.5, 0.367916-0.5, 0.838477 + 0.239713)
hold on
nv2 = quiver3(0,0,0,0, 0.447214, 0.894427)
hold on
nv3 = quiver3(0,0,0,-0.401987, 0.367916, 0.838477)
hold on
nv4 = quiver3(0,0,0,0.43231, -0, 0.901725)
hold on
nv5 = quiver3(0,0,0,0, 0, 1)
hold on
nv6 = quiver3(0,0,0,-0.43231, -0, 0.901725)
hold on
nv7 = quiver3(0,0,0,0.401987, -0.367916, 0.838477)
hold on
nv8 = quiver3(0,0,0,-0, -0.447214, 0.894427)
hold on
nv9 = quiver3(0,0,0,-0.401987, -0.367916, 0.838477)
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

textx = text(0.401987, 0.367916, 0.838477, 'normal 1')
set(textx, 'Color', 'blue')