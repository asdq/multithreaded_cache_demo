program fms;

{$mode objfpc}{$H+}

uses
  {$IFDEF UNIX}{$IFDEF UseCThreads}
  cthreads,
  {$ENDIF}{$ENDIF}
  Interfaces, // this includes the LCL widgetset
  Forms, MSMain;

begin
  Application.Initialize;
  Application.CreateForm(TTMSMainForm, TMSMainForm);
  Application.Run;
end.

