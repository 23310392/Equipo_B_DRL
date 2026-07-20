function xdot = KE88_dinamica(u)
%KE88_DINAMICA Dinamica de cuerpo rigido 6DOF del dron KE88.
% Este es el corazon fisico del "gemelo digital": recibe el estado actual
% y los empujes de los 4 motores, y devuelve la derivada del estado
% (Simulink la integra con el bloque Integrator).
%
% Entrada u (16x1):
%   u(1:12) = estado x = [pos(3); vel(3); euler(3); omega_cuerpo(3)]
%   u(13:16)= empujes de motores [T1;T2;T3;T4] en Newtons
% Salida xdot (12x1) = derivada del estado
%
% Ecuaciones (Newton-Euler):
%   m*dv/dt   = -m*g*z + R*[0;0;Ftotal] - kd*v      (traslacion, marco mundo)
%   I*dw/dt   = tau - w x (I*w)                     (rotacion, marco cuerpo)
%   deuler/dt = W(phi,theta) * w                    (cinematica de Euler)

global P
if isempty(P)
    KE88_parametros;   % carga los parametros si aun no existen
end

% --- desempacar ---
x   = u(1:12);
T   = u(13:16);
pos = x(1:3);   v   = x(4:6);
eul = x(7:9);   om  = x(10:12);
phi = eul(1); th = eul(2); psi = eul(3);

% --- matriz de rotacion cuerpo->mundo (Euler ZYX, z arriba) ---
cph=cos(phi); sph=sin(phi);
cth=cos(th);  sth=sin(th);
cps=cos(psi); sps=sin(psi);
R = [ cps*cth,  cps*sth*sph - sps*cph,  cps*sth*cph + sps*sph ;
      sps*cth,  sps*sth*sph + cps*cph,  sps*sth*cph - cps*sph ;
     -sth    ,  cth*sph              ,  cth*cph               ];

% --- fuerzas (marco mundo) ---
Ftot = sum(T);
acc  = [0;0;-P.g] + R*[0;0;Ftot]/P.m - (P.kd_lin/P.m)*v;

% --- pares (marco cuerpo) ---
tau = [ P.dy*( T(1) - T(2) - T(3) + T(4));    % alabeo
       -P.dx*( T(1) + T(2) - T(3) - T(4));    % cabeceo
        P.c *(-T(1) + T(2) - T(3) + T(4))]... % guinada
      - P.kd_ang*om;

I     = diag([P.Ixx P.Iyy P.Izz]);
omdot = I \ (tau - cross(om, I*om));

% --- cinematica de Euler (omega cuerpo -> derivadas de angulos) ---
W = [1, sph*tan(th), cph*tan(th);
     0, cph        , -sph       ;
     0, sph/cth    , cph/cth    ];
euldot = W*om;

xdot = [v; acc; euldot; omdot];

% --- suelo simple: el dron no atraviesa el piso ---
if pos(3) <= 0
    xdot(3) = max(v(3), 0);       % no bajar mas
    xdot(6) = max(acc(3), 0);     % no acelerar hacia abajo
    if v(3) <= 0                  % apoyado: se frena y se auto-nivela
        xdot(4:5)   = -5*v(1:2);              % friccion horizontal
        xdot(10:12) = -10*om;                  % frena la rotacion
        xdot(7:9)   = [-2*phi; -2*th; 0];      % se nivela (yaw se conserva)
    end
end
end
