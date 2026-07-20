function KE88_animacion(out)
%KE88_ANIMACION Reproduce en 3D el vuelo simulado del KE88.
% Uso: despues de correr la simulacion en Simulink, escribe en la consola:
%       KE88_animacion(out)
% ("out" es la variable que Simulink deja en el workspace con los datos)

if nargin < 1
    out = evalin('base','out');
end
t = out.x_log.time;
X = out.x_log.signals.values;      % N x 12

global P
if isempty(P), KE88_parametros; end

% --- geometria del dron para dibujar ---
b = 1.6;                            % factor visual (agranda el dron)
m1 = b*[ P.dx;  P.dy; 0];  m2 = b*[ P.dx; -P.dy; 0];
m3 = b*[-P.dx; -P.dy; 0];  m4 = b*[-P.dx;  P.dy; 0];

fig = figure('Name','KE88 - Gemelo Digital','Color','w');
ax  = axes('Parent',fig); hold(ax,'on'); grid(ax,'on'); view(ax,3);
xlabel('x [m]'); ylabel('y [m]'); zlabel('z [m]');
title('Simulacion KE88');

% suelo
[gx,gy] = meshgrid(-3:0.5:3);
surf(ax,gx,gy,0*gx,'FaceColor',[0.92 0.95 0.92],'EdgeColor',[0.8 0.85 0.8]);

h_arm1  = plot3(ax,nan,nan,nan,'k-','LineWidth',3);            % brazo M1-M3
h_arm2  = plot3(ax,nan,nan,nan,'k-','LineWidth',3);            % brazo M2-M4
h_props = plot3(ax,nan,nan,nan,'o','MarkerSize',9,...
    'MarkerFaceColor',[0.2 0.6 0.3],'MarkerEdgeColor','k');
h_nose  = plot3(ax,nan,nan,nan,'r.','MarkerSize',20);          % frente
h_trail = plot3(ax,nan,nan,nan,'b-','LineWidth',1);            % estela

% submuestreo para ~25 cuadros por segundo de vuelo
paso = max(1, round(numel(t)/(t(end)*25)));

for k = 1:paso:numel(t)
    if ~isvalid(fig), return; end
    pos = X(k,1:3)';  eul = X(k,7:9)';
    R = rotZYX(eul);
    p1=pos+R*m1; p2=pos+R*m2; p3=pos+R*m3; p4=pos+R*m4;
    nose = pos + R*[b*P.dx*1.4;0;0];

    set(h_arm1,'XData',[p1(1) p3(1)],'YData',[p1(2) p3(2)],'ZData',[p1(3) p3(3)]);
    set(h_arm2,'XData',[p2(1) p4(1)],'YData',[p2(2) p4(2)],'ZData',[p2(3) p4(3)]);
    set(h_props,'XData',[p1(1) p2(1) p3(1) p4(1)],...
                'YData',[p1(2) p2(2) p3(2) p4(2)],...
                'ZData',[p1(3) p2(3) p3(3) p4(3)]);
    set(h_nose,'XData',nose(1),'YData',nose(2),'ZData',nose(3));
    set(h_trail,'XData',X(1:k,1),'YData',X(1:k,2),'ZData',X(1:k,3));

    % la camara sigue al dron
    axis(ax,[pos(1)-1.5 pos(1)+1.5 pos(2)-1.5 pos(2)+1.5 0 max(2,pos(3)+1)]);
    title(ax,sprintf('KE88  t = %.1f s   altura = %.2f m',t(k),pos(3)));
    drawnow limitrate
end
end

function R = rotZYX(eul)
phi=eul(1); th=eul(2); psi=eul(3);
cph=cos(phi); sph=sin(phi); cth=cos(th); sth=sin(th); cps=cos(psi); sps=sin(psi);
R = [ cps*cth,  cps*sth*sph - sps*cph,  cps*sth*cph + sps*sph ;
      sps*cth,  sps*sth*sph + cps*cph,  sps*sth*cph - cps*sph ;
     -sth    ,  cth*sph              ,  cth*cph               ];
end
