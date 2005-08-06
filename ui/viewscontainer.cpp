#include <klocale.h>
#include <kiconloader.h>

#include "mainview.h"
#include "viewscontainer.h"


ViewsContainer::ViewsContainer(QWidget * parent)
    : QWidget(parent)
{
    //Bottom bar layout
    m_HLayout = new QHBoxLayout();
    m_HLayout->setSpacing(0);
    m_HLayout->setMargin(0);
    


    m_downloads = new QToolButton();
    m_downloads->setToolButtonStyle ( Qt::ToolButtonTextBesideIcon );
    m_downloads->setText(i18n("Downloads"));
    m_downloads->setIcon(SmallIcon("kget"));

    m_finished = new QToolButton();
    m_finished->setToolButtonStyle ( Qt::ToolButtonTextBesideIcon );
    m_finished->setText(i18n("Finished"));
    m_finished->setIcon(SmallIcon("ok"));

    m_HLayout->addWidget(m_downloads);
    m_HLayout->addWidget(m_finished);
    m_HLayout->addStretch(1);

    //

    m_VLayout = new QVBoxLayout();
    m_VLayout->setSpacing(1);
    m_VLayout->setMargin(0);
    setLayout(m_VLayout);

    m_mainView = new MainView();
    m_VLayout->addWidget(m_mainView);
    m_VLayout->addLayout(m_HLayout);
}


