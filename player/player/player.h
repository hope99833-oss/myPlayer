#pragma once

#include <QtWidgets/QWidget>
#include "ui_player.h"

class player : public QWidget
{
    Q_OBJECT

public:
    player(QWidget *parent = nullptr);
    ~player();

private:
    Ui::playerClass ui;
};

