program Project1;

uses
  Forms,
  Unit1 in 'Unit1.pas' {Form1},
  XPTheme in 'XPTheme.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.Title := 'Miranda Setup Wizard';
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
