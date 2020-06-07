#include "behavior_aux.h"
#include "time/QTime.h"

namespace htmlayout 
{

bool belongs_to( dom::element parent, dom::element child );
using namespace htmlayout::dom;

struct littlet_plantitle: public behavior
{
    // ctor
    littlet_plantitle()
        : behavior(HANDLE_MOUSE | HANDLE_FOCUS | HANDLE_BEHAVIOR_EVENT ,
            "littlet_plantitle") 
    {

    }

    virtual void attached  (HELEMENT he ) 
    { 
		element r = he;
		r.append(element::create("caption",L"00:00:00"));
		element etable = element::create("table");
		r.append(etable);
		etable.set_attribute("time",L"00:00:00");
	} 

	virtual BOOL on_mouse(HELEMENT he, HELEMENT target, UINT event_type, 
		POINT pt, UINT mouseButtons, UINT keyboardStates )
	{
		switch(event_type)
		{
		case MOUSE_DOWN:
			{
				element er = he;
				HELEMENT hPopup = er.find_first("table");
				if( element(hPopup).is_valid() && er.is_valid() && !er.get_state(STATE_OWNS_POPUP) )
				{
					::HTMLayoutShowPopup(hPopup,er,2); // show it below
				}
				return true;
			}
		}
		return false;
	}

	virtual BOOL on_event (HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason ) 
	{ 
		switch(type)
		{
		case POPUP_DISMISSED:
			{
				dom::element eSel = element(target).find_first("td:checked");
				element ecaption = element(he).find_first("caption");
				if (eSel.is_valid() && ecaption.is_valid())
				{
					json::string s = aux::a2w(eSel.get_html(false).c_str());
					ecaption.set_value(json::value(s));
					element(he).send_event(SELECT_SELECTION_CHANGED);
					element(target).set_attribute("time", s.c_str());
				}
				return TRUE;
			}
		}
		return FALSE;
	}
	
};

// instantiating and attaching it to the global list
littlet_plantitle xtimectrl_instance;
xtimepopup xtimectrl_popup_instance;
}
