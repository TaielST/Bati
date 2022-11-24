#include "ButtonRyo.h"""

Button::Button(int p)
{
    pin = p;
    pinMode(pin, INPUT);
}

void Button::SetFlanco(bool f)
{
    flank = f;
    previousState = !flank;
}

bool Button::GetIsPress()
{
    bool actualState = digitalRead(pin);
    bool State = (previousState != actualState) && actualState == flank;
    previousState = actualState;
    delay(100);
    return State;
}

bool Button::SwitchOrStar()
{
    if(GetIsPress())
    {
        cont = 0;
        while(digitalRead(pin))
        {
            cont++;
        }
        if(cont < 1000) return 1;
        if(cont > 1000) return 0;
    }
}