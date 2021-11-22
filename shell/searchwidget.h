/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "./utils/utils.h"

#include <QWidget>
#include <QList>
#include <QFile>
#include <QEvent>
#include <QLocale>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStandardItemModel>
#include <QXmlStreamReader>

const QString XML_Source       = "source";
const QString XML_Title        = "translation";
const QString XML_Numerusform  = "numerusform";
const QString XML_Explain_Path = "extra-contents_path";

class SearchWidget : public QLineEdit
{
    Q_OBJECT
public:
    struct SearchBoxStruct {
        QString translateContent;
        QString actualModuleName;
        QString childPageName;
        QString fullPagePath;
    };

    struct SearchDataStruct {
        QString chiese;
        QString pinyin;
    };

public:
    SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget() override;

    bool jumpContentPathWidget(QString path);
    void setLanguage(QString type);
    void addModulesName(QString moduleName, QString searchName, QString translation = "");

private Q_SLOTS:
    void onCompleterActivated(QString value);

Q_SIGNALS:
    void notifyModuleSearch(QString, QString);

private:
    void loadxml();
    SearchBoxStruct getModuleBtnString(QString value);
    QString getModulesName(QString name, bool state = true);
    QString removeDigital(QString input);
    QString transPinyinToChinese(QString pinyin);
    QString containTxtData(QString txt);
    void appendChineseData(SearchBoxStruct data);
    void clearSearchData();

    void initExcludeSearch();


private:
    QStandardItemModel *m_model;
    QCompleter *m_completer;
    QList<SearchBoxStruct> m_EnterNewPagelist;
    SearchBoxStruct m_searchBoxStruct;
    QString m_xmlExplain;
    QSet<QString> m_xmlFilePath;
    QString m_lang;
    QList<QPair<QString, QString>> m_moduleNameList;//用于存储如 "update"和"Update"
    QList<SearchDataStruct> m_inputList;
    bool m_bIsChinese;
    QString m_searchValue;
    bool m_bIstextEdited;
    QStringList m_defaultRemoveableList;//存储已知全部模块是否存在
    QList<QString> m_TxtList;

    QStringList mCnExclude;
    QStringList mEnExclude;
};
#endif // SEARCHWIDGET_H
