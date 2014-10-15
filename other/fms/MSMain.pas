{ Author: Fabio Vaccari, fabio.vaccari@gmail.com
  Released under MIT license

Copyright (c) 2011 Vaccari Fabio

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
}

// see http://www.freepascal.org and the Lazarus project
unit MSMain;

{ Draw a square of numbers from 1 to n*n and calculate the sum of each row,
column and diagonal. If all the sum are equal, then it is a magic square.

Buttons mapping:

          0, 1,   ...   cDim-1,        cDim,       cDim+1,     ...   MAXDIM
0        [<------ Empty ----->][   sum of diag2   ]<----- not visible ----->]
1        [<--- first row ---->][ sum of first row ] ...                 ... ]
2         ...                   ...                 ...                 ...
...       ...                   ...                 ...                 ...
cDim     [<---- last row ---->][ sum of last row  ] ...                 ... ]
cDim+1   [<- sum of columns ->][   sum of diag1   ] ...                 ... ]
cDim+2   [<-------------------------- not visible ------------------------->]
...                                      ...
MAXDIM+1 [...                            ...                            ... ]

Flow:
  TmsMainForm.OnCreate ->InitForm[GetLanguage,
    SetDim[SetButtons, ->CheckSquare[Calculate], ->HeadPanelClick]]
  DimSpinEdit.OnChange ->DimSpinChanged[
    SetDim[SetButtons, ->CheckSquare[Calculate], ->HeadPanelClick]
  RestartButton.OnClick ->RestartButtonClick[
    SetButtons, ->CheckSquare[Calculate], ->HeadPanelClick]
  HeadPanel.OnClick ->HeadPanelClick
  MessageLabel.OnClick ->HeadPanelClick

  button.OnClick ->ButtonClicked[ ->CheckSquare[Calculate]]
}

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, LResources, Forms, Graphics,
  ExtCtrls, StdCtrls, Spin, Dialogs, GetText;

const MAXDIM = 7; MINDIM = 3; // bounds of the square

type
  TButtonTbl = class(TButton)
  private
    tbRow, tbCol, tbVal:Integer;
    function GetRow:Integer; procedure SetRow(aRow:Integer);
    function GetCol:Integer; procedure SetCol(aCol:Integer);
    function GetVal:Integer; procedure setVal(aVal:Integer);
  public
    property Row:Integer read GetRow write SetRow;
    property Col:Integer read GetCol write SetCol;
    property Value:Integer read GetVal write SetVal;
  end;
  PTButtonTbl = ^TButtonTbl;

  { TTMSMainForm }

  TTMSMainForm = class(TForm)
    RestartButton: TButton;
    HeadPanel: TPanel;
    BasePanel: TPanel;
    DimLabel: TLabel;
    DimSpinEdit: TSpinEdit;
    MessageLabel: TLabel;
    procedure HeadPanelClick(Sender: TObject);
    procedure InitForm(Sender:TObject);
    procedure CheckSquare(Sender:TObject);
    procedure DimSpinChanged(Sender:TObject);
    procedure ButtonClicked(Sender:TObject);
    procedure RestartButtonClick(Sender: TObject);
  private
    authorStr, versionStr, licenceStr, contactStr, restartButtonStr:String;
    dimLabelStr, msgMagicStr, msgNotMagicStr, helpStr, titleStr:String;

    baseTable:array[0..MAXDIM+1,0..MAXDIM] of TButtonTbl;
    cDim:Integer; // current dimension of the square
    lRow, lCol:Integer; lCap:String; // last button selected
    procedure GetLanguage;
    procedure SetButtons(aDim:Integer);
    function Calculate:boolean;
    function GetDim:Integer; procedure SetDim(aDim:Integer);
  public
    property Dim:Integer read getDim write setDim;
  end;

var
  TMSMainForm: TTMSMainForm;

implementation

  function TButtonTbl.GetRow:Integer; begin GetRow := tbRow; end;
  procedure TButtonTbl.SetRow(aRow:Integer); begin tbRow := aRow; end;
  function TButtonTbl.GetCol:Integer; begin GetCol := tbCol; end;
  procedure TButtonTbl.SetCol(aCol:Integer); begin tbCol := aCol; end;
  function TButtonTbl.GetVal:Integer; begin GetVal := tbVal; end;
  procedure TButtonTbl.setVal(aVal:Integer); begin tbVal := aVal; end;

  procedure TTMSMainForm.InitForm(Sender:TObject);
  var i, j:Integer;
  begin
    GetLanguage;
    Self.Caption := titleStr; DimLabel.Caption := dimLabelStr;
    RestartButton.Caption := restartButtonStr;
    DimSpinEdit.MaxValue := MAXDIM; DimSpinEdit.MinValue := MINDIM;
    BasePanel.Visible := false;
    BasePanel.ChildSizing.ControlsPerLine := MAXDIM;
    for i := 0 to MAXDIM+1 do
    begin
      for j := 0 to MAXDIM do
      begin
        baseTable[i, j] := TButtonTbl.Create(BasePanel);
        baseTable[i, j].Parent := BasePanel;
        baseTable[i, j].Visible := false;
        baseTable[i, j].Row := i;
        baseTable[i, j].Col := j;
        baseTable[i, j].Value := 0;
        baseTable[i, j].OnClick := @ButtonClicked;
        baseTable[i, j].Color := clAqua;
        baseTable[i, j].Font.Style := [fsBold];
      end;
    end;
    cDim := 0; SetDim(3);
  end;

  procedure TTMSMainForm.HeadPanelClick(Sender: TObject);
  begin
    if MessageLabel.Caption = licenceStr then
      MessageLabel.Caption := contactStr
    else if MessageLabel.Caption = authorStr then
      MessageLabel.Caption := licenceStr
    else if MessageLabel.Caption = versionStr then
      MessageLabel.Caption := authorStr
    else if messageLabel.Caption = helpStr then
      MessageLabel.Caption := versionStr
    else messageLabel.Caption := helpStr;
  end;

  procedure TTMSMainForm.CheckSquare(Sender:TObject);
  var c:Integer; s:String;
  begin
    if Calculate then s := msgMagicStr
    else s := msgNotMagicStr;
    MessageLabel.Caption := s;

    s := ' = ' + IntToStr(baseTable[0, cdim].Value);
    baseTable[0,cDim].Caption := s;

    s := ' = ' + IntToStr(baseTable[cDim+1,cDim].Value);
    baseTable[cDim+1,cDim].Caption := s;

    for c := 1 to cDim do
    begin
      s := ' = ' + IntToStr(baseTable[c, cDim].Value);
      baseTable[c, cDim].Caption := s;
    end;

    for c := 0 to cDim-1 do
    begin
      s := ' = ' + IntToStr(baseTable[cDim+1, c].Value);
      baseTable[cDim+1, c].Caption := s;
    end;
  end;

  function TTMSMainForm.GetDim:Integer;
  begin
    GetDim := cDim;
  end;

  procedure TTMSMainForm.DimSpinChanged(Sender:TObject);
  begin
    Dim := DimSpinEdit.Value;
  end;

  procedure TTMSMainForm.ButtonClicked(Sender:TObject);
  var sb:PTButtonTbl;
  begin
    if Sender is TButtonTbl then
    begin
      sb := @Sender;
      if (0<sb^.Row) AND (sb^.Row<cDim+1) AND (sb^.Col<cDim) then
      begin
        if lCap = '' then
        begin
          lRow := sb^.Row; lCol := sb^.Col; lCap := sb^.Caption;
          sb^.Color := clSilver;
        end
        else
        begin
          if (lRow<>sb^.Row) OR (lCol<>sb^.Col) then
          begin
            baseTable[lRow, lCol].Value := sb^.Value;
            baseTable[lRow, lCol].Caption := sb^.Caption;
            sb^.Value:= StrToInt(lCap);
            sb^.Caption := lCap;
            CheckSquare(Sender);
          end;
          baseTable[lRow, lCol].Color := clWhite;
          lRow := 0; lCol := 0; lCap := '';
        end;
      end;
    end;
  end;

  procedure TTMSMainForm.RestartButtonClick(Sender: TObject);
  begin
    SetButtons(cDim); CheckSquare(Sender); HeadPanelClick(Sender);
  end;

  procedure TTMSMainForm.SetDim(aDim:Integer);
  begin
    if (cDim<>aDim) AND (MINDIM<=aDim) AND (aDim<=MAXDIM) then
    begin
      SetButtons(aDim); cDim := aDim;
      CheckSquare(self); HeadPanelClick(self);
    end
  end;

  procedure TTMSMainForm.SetButtons(aDim:Integer);
  var i, j, count:Integer;
  begin
    lRow := 0; lCol := 0; lCap := '';
    BasePanel.Visible := false;
    BasePanel.ChildSizing.ControlsPerLine := aDim+1;
    count := 1;
    for i := 0 to MAXDIM+1 do
    begin
      for j := 0 to MAXDIM do
      begin
        if (i<=aDim+1) AND (j<=aDim) then
        begin
          baseTable[i, j].Visible := true;
          if (0<i) AND (i<aDim+1) AND (j<aDim) then
          begin // the square
            baseTable[i, j].Value := count;
            baseTable[i, j].Caption := IntToStr(count);
            baseTable[i, j].Color := clWhite;
            Inc(count);
          end
          else
          begin
            baseTable[i, j].Value := 0;
            baseTable[i, j].Caption := '   ';
            baseTable[i, j].Color := clAqua;
          end;
        end
        else
        begin
          baseTable[i, j].Visible := false;
          baseTable[i, j].Value := 0;
          baseTable[i, j].Caption := '';
          baseTable[i, j].Color := clAqua;
        end;
      end;
    end;
    BasePanel.Visible := true;
  end;

  function TTMSMainForm.Calculate:boolean;

    var
      magic:Boolean;
      x, row, col, k:Integer; // x is from 1 to cDim*cDim, k is the magic const
      //checkList:TBits; // checkList[x-1] = true if x was already parsed

    //procedure CkX(x:Integer);
    //var i:Integer;
    //begin
    //  i := x-1;
    //  if (0<=i) AND (i<dim*dim) AND NOT checkList[i]
    //  then checkList[i] := true
    //  else magic := false;
    //end;

    procedure CkSum(s:Integer);
    begin
      if s<>k then magic := false;
    end;

    //procedure InitCheckList(aLen:Integer);
    //var i:Integer;
    //begin
    //  checkList := TBits.Create; checkList.Size := aLen;
    //  for i := 0 to aLen-1 do checkList[i] := false;
    //end;

    procedure InitSums;
    var i:Integer;
    begin
      for i := 0 to cDim do
      begin
        baseTable[i, cDim].Value := 0;
        baseTable[cDim+1, i].Value := 0;
      end;
    end;
    
  begin
    magic := true; // suppose it is a magic square
    k := (cDim*cDim*cDim + cDim) DIV 2;
    //InitCheckList(cDim*cDim); 
    InitSums;
    for row := 1 to cDim do
    begin
      for col := 0 to cDim-1 do
      begin
        x := baseTable[row, col].Value;
        baseTable[row, cDim].Value := baseTable[row, cDim].Value + x;
        baseTable[cDim+1, col].Value := baseTable[cDim+1, col].Value + x;
        //CkX(x);
        if cDim-row = col then // second diagonal
          baseTable[0, cdim].Value := baseTable[0, cDim].Value + x;
        if row-1 = col then // first diagonal
          baseTable[cDim+1,cDim].Value := baseTable[cDim+1,cDim].Value + x;
        if row = cDim then // end of each column
          CkSum(baseTable[cDim+1, col].Value);
      end;
      CkSum(baseTable[row, cDim].Value); // end of each row
    end;
    CkSum(baseTable[0, cdim].Value); CkSum(baseTable[cDim+1,cDim].Value);
    Calculate := magic;
  end;

  procedure TTMSMainForm.GetLanguage; // embed, no i18n
  var lang, fallbackLang: String;
  begin
    lang := ''; fallbackLang := '';
    GetLanguageIDs(Lang, fallbackLang);
    if fallbackLang = 'it' then
    begin
      authorStr := ' Scritto da Vaccari Fabio ';
      contactStr := ' Mail: fabio.vaccari@gmail.com ';
      licenceStr := ' Licenza: MIT ';
      restartButtonStr := 'Nuovo';
      dimLabelStr := ' Dim:';
      msgMagicStr := ' Hai trovato un quadrato magico! ';
      msgNotMagicStr := ' Non è un quadrato magico. ';
      helpStr := 'Sposta i numeri della tabella.';
      titleStr := ' Quadrati Magici ';
    end
    else if fallbackLang = 'ru' then
    begin
      authorStr := ' Автор: Ваккари Фабио ';
      contactStr := ' Контакт: fabio.vaccari@gmail.com ';
      licenceStr := ' Лицензия: MIT ';
      restartButtonStr := 'Снова';
      dimLabelStr := ' Размер:';
      msgMagicStr := ' Вы нашли магический квадрат! ';
      msgNotMagicStr := ' Это не магический квадрат. ';
      helpStr := ' Переместите номера в квадрате. ';
      contactStr := ' Contact: fabio.vaccari@gmail.com ';
      titleStr := ' Магические Квадраты ';
    end
    else
    begin
      authorStr := ' Author: Vaccari Fabio ';
      contactStr := ' Contact: fabio.vaccari@gmail.com ';
      licenceStr := ' License: MIT ';
      restartButtonStr := 'Restart';
      dimLabelStr := ' Dim:';
      msgMagicStr := ' You found a magic square! ';
      msgNotMagicStr := ' This is not a magic square. ';
      helpStr := ' Move numbers in the square. ';
      titleStr := ' Magic Squares ';
    end;
    versionStr := ' Find Magic Squares v.1 ';
  end;

initialization

  {$I MSMain.lrs}

end.

