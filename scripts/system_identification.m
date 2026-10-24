% Aufräumen
clear all
clc
close all


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                       KONFIGURATION                      %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Rohdatei
filename = '../data/step_response_0_to_50.log';

% Feste Parameter (Totzeit über dead_time_estimation.py)
T0 = 1  % Abtastzeit
Tt = 31 % Totzeit


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                           EINLESEN                       %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Gesamten Text der Datei als String einlesen
fileText = fileread(filename);

% Regex definieren, um die relevanten Daten zu extrahieren
pattern = 'StepResp:(\d+)\s+Ist:(\d+)\s+Soll:\d+\s+([0-9.]+)%';
tokens = regexp(fileText, pattern, 'tokens');

% Text in eine numerische Matrix umwandeln
dataList = cellfun(@(x) [str2double(x{1}), str2double(x{3}), str2double(x{2})], tokens, 'UniformOutput', false);
data_raw = vertcat(dataList{:});

% Daten filtern
if ~isempty(data_raw)
    % Finde erste Zeile, in der StepResp genau 0 ist (Start der Messung)
    startIndex = find(data_raw(:,1) == 0, 1, 'first');
    
    if ~isempty(startIndex)
        % Alle Daten vor StepResp:0 abschneiden
        data = data_raw(startIndex:end, :);
        
        % Ende der Messung erkennen (sobald sich StepResp wiederholt)
        % diff(data(:,1)) gibt 0 zurück, wenn zwei aufeinanderfolgende Werte gleich sind
        stopIndex = find(diff(data(:, 1)) == 0, 1, 'first');
        
        if ~isempty(stopIndex)
            % Alle Daten nach dem ersten Auftreten des wiederholten Werts abschneiden
            data = data(1:stopIndex, :);
        end
        
        % Zuweisung
        zeit = data(:, 1);        
        eingang = data(:, 2);     
        temperatur = data(:, 3);  
        
        fprintf('Daten erfolgreich eingelesen! %d gültige Messpunkte geladen.\n', length(zeit));
    else
        disp('Fehler: Start der Messung (StepResp:0) wurde in der Datei nicht gefunden.');
    end
else
    disp('Fehler: Keine passenden Datenzeilen in der Datei gefunden.');
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                    MESSUNG DARSTELLEN                    %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Plotten
figure(1);
plot(zeit, temperatur);
xlabel('Zeit [s]');
ylabel('Temperatur [0,01 °C]');
title('Sprungantwort');
grid on;
xscale linear;
ax = gca;
ax.XAxis.Exponent = 0;
ax.YAxis.Exponent = 0;

% Trennlinien und Beschriftung
idx_vorne = find(diff(eingang) ~= 0, 1, 'first');
idx_hinten = find(diff(eingang) ~= 0, 1, 'last');
xline(zeit(idx_vorne), '--black', 'LineWidth', 0.5);
xline(zeit(idx_hinten), '--black', 'LineWidth', 0.5);
x1 = zeit(1);
x2 = zeit(idx_vorne);
x3 = zeit(idx_hinten);
x4 = zeit(length(zeit));
yl = ylim;
y_text = (yl(1) + yl(2)) / 2;
text((x1+x2)/2, y_text, [num2str(eingang(idx_vorne-1)) ' %'], 'FontSize', 24, 'HorizontalAlignment', 'center', 'Color', 'black');
text((x2+x3)/2, y_text, [num2str(eingang(idx_hinten-1)) ' %'], 'FontSize', 24, 'HorizontalAlignment', 'center', 'Color', 'black');
text((x3+x4)/2, y_text, [num2str(eingang(end)) ' %'], 'FontSize', 24, 'HorizontalAlignment', 'center', 'Color', 'black');

% Speichern
exportgraphics(gcf, '../doc/figures/step_response.png', 'Resolution', 300);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                   SYSTEMIDENTIFIKATION                   %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Daten als Spaltenvektoren
temperatur_sv = temperatur(:);
eingang_sv = eingang(:);

% Konstanten Offset-Eingang
eingang_offset = ones(size(eingang_sv));
eingangs_matrix = [eingang_sv, eingang_offset]; 

% Trainingsdaten
data = iddata(temperatur_sv, eingangs_matrix, T0);
data.InputName = {'Heizleistung', 'Offset'};

% Modellstruktur
init_sys = idproc({'P1D', 'P0'});

% Totzeit für ersten Eingang festsetzen
init_sys.Structure(1).Td.Value = Tt;
init_sys.Structure(1).Td.Free = false;

% Warnungen deaktiveren, da Offset kein Sprung hat
warning('off', 'all')

% Schätzung mit fixierter Totzeit
sys = pem(data, init_sys);

% Warnungen aktivieren
warning('on', 'all')


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                           AUSGABE                        %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Vergleichsdiagramm
fig2 = figure(2);
compare(data, sys)

% Beschriftung, Axen und Farben
xlabel('Zeit [s]');
ylabel('Temperatur [0,01 °C]');
title('Sprungantwort: Vergleich Messung und Modell');
txt = findall(fig2, 'Type', 'text', 'String', 'y1');
delete(txt);
txt = findall(fig2, 'Type', 'text', 'String', '(seconds)');
delete(txt);
ax = findall(gcf, 'Type', 'axes');
ax(4).XAxis.Exponent = 0;
ax(1).XLabel.String = 'Zeit [s]';
h = findall(gcf, 'Type', 'line');
h(7).Color = h(6).Color;
h(6).Color = [0.8500 0.3250 0.0980];

% Linien und Beschriftung
xline(zeit(idx_vorne), '--black', 'LineWidth', 0.5);
xline(zeit(idx_hinten), '--black', 'LineWidth', 0.5);
yl = ylim;
y_text = (yl(1) + yl(2)) / 2;
text((x1+x2)/2, y_text, [num2str(eingang(idx_vorne-1)) ' %'], 'FontSize', 24, 'HorizontalAlignment', 'center', 'Color', 'black');
text((x2+x3)/2, y_text, [num2str(eingang(idx_hinten-1)) ' %'], 'FontSize', 24, 'HorizontalAlignment', 'center', 'Color', 'black');
text((x3+x4)/2, y_text, [num2str(eingang(end)) ' %'], 'FontSize', 24, 'HorizontalAlignment', 'center', 'Color', 'black');

% Legende
[y, fit] = compare(data, sys);
lgd = findall(gcf, 'Type', 'legend');
delete(lgd);
hold on;
p1 = plot(nan, nan, '-', 'Color', h(7).Color, 'LineWidth', 1.5);
p2 = plot(nan, nan, '-', 'Color', h(6).Color, 'LineWidth', 1.5);
legend([p1 p2], {'Messung', ['PT1-Tt-Modell: ' num2str(fit) '%']}, 'Location', 'best');
hold off;

% Speichern
exportgraphics(gcf, '../doc/figures/step_response_model_comparison.png', 'Resolution', 300);

% Ausgabe Modell
sys

% Ausgabe der trainierten Parameter
disp('--- Trainierte Modellparameter ---')
K = sys.Kp(1)          % Streckenverstärkung
T1 = sys.Tp1(1)        % Zeitkonstante
T_offset = sys.Kp(2)   % Dynamisch trainierter Offset

% Speichern
fid = fopen('../data/system_parameters_estimation.txt', 'w');

% Parameter hineinschreiben
fprintf(fid, 'T1 = %.11f\n', T1);
fprintf(fid, 'K = %.11f\n', K);
fprintf(fid, 'Tt = %.11f\n', Tt);
fprintf(fid, 'T0 = %.11f\n', T0*1000);

% Datei schließen
fclose(fid);

% Alle unnötigen Hilfsvariablen löschen
clearvars -except sys T1 K Tt T0 T_offset