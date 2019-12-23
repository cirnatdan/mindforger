/*
 dashboard_presenter.cpp     MindForger thinking notebook

 Copyright (C) 2016-2019 Martin Dvorak <martin.dvorak@mindforger.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "dashboard_presenter.h"

namespace m8r {

using namespace std;

DashboardPresenter::DashboardPresenter(DashboardView* view, OrlojPresenter* orloj)
{
    this->view = view;

    organizerDashboardletPresenter = new OrganizerPresenter(
                view->getOrganizerDashboardlet(),
                orloj);
    recentDashboardletPresenter = new RecentNotesTablePresenter(
                view->getRecentDashboardlet(),
                orloj->getMainPresenter()->getHtmlRepresentation());
    navigatorDashboardletPresenter = new NavigatorPresenter(
                view->getNavigatorDashboardlet(),
                this,
                orloj->getMind()->getKnowledgeGraph());
}

DashboardPresenter::~DashboardPresenter()
{
}

void DashboardPresenter::refresh(const vector<Outline*>& os, const vector<Note*>& ns)
{
    organizerDashboardletPresenter->refresh(os);
    recentDashboardletPresenter->refresh(ns);
    navigatorDashboardletPresenter->showInitialView();
}

} // m8r namespace