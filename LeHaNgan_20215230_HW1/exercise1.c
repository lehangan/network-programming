#include<stdio.h>
#include<string.h>

void print(char *a, char *b, char *c, char* d, char* e)
{
    printf("***Emergency action advice***\n");
    printf("Material: %s\n", a);
    printf("*****************************\n");
}
int main()
{
    char str[5];
    char rev[4];
    int color;
    char material[10], reactivity[200], protection[200], containment[200], evacuation[200];
    printf("Enter HAZCHEM code: ");
    scanf("%s", str);
    if(str[1] == 'S' || str[1] == 'Y' || str[1] == 'Z' || str[1] == 'T')
    {
        printf("Is the %c reverse coloured? ", str[1]);
        scanf("%s", rev);
        //fflush(stdin);
        if(!strcmp(rev, "yes")) color = 1 ;
        else color = 0;
        printf("%c\n", str[1] );
    }
    if( (str[0] - 48 )== 1) strcpy(material, "jets");
    else if( ( str[0]-48 ) == 2 ) strcpy( material, "fog");
    else if( ( str[0]-48 ) == 3 ) strcpy( material, "foam");
    else strcpy( material, "dry agent");
    if( str[2] == 'E' )
        strcpy(evacuation,"consider Evacuation");
    else strcpy(evacuation,"");

    if(str[1] == 'P' )
    {
        strcpy(reactivity,"can be violently reactive");
        strcpy(protection, "full protective clothing must be worn");
        strcpy(containment, "may be washed down to a drain with a large quantity of water");
    }
    else if(str[1] == 'S')
    {
        strcpy(reactivity,"can be violently reactive");
        strcpy(containment, "may be washed down to a drain with a large quantity of water");
        if(color == 1)
            strcpy(protection,"breathing apparatus, protective gloves for fire only");
        else strcpy(protection, "full protective clothing must be worn");
    }
    else if(str[1] == 'T')
    {
        strcpy(reactivity,"");
        strcpy(containment, "may be washed down to a drain with a large quantity of water");
        if(color == 1)
            strcpy(protection,"breathing apparatus, protective gloves for fire only");
        else strcpy(protection, "full protective clothing must be worn");
    }
    else if( str[1] =='W')
    {
        strcpy(reactivity,"can be violently reactive");
        strcpy(protection, "full protective clothing must be worn");
        strcpy(containment, "a need to avoid spillages from entering drains or water courses");
    }
    else if( str[1] =='X')
    {
        strcpy(reactivity,"");
        strcpy(protection, "full protective clothing must be worn");
        strcpy(containment, "may be washed down to a drain with a large quantity of water");
    }
    else if(str[1] == 'Y' || str[1] == 'Z')
    {
        strcpy(reactivity,"can be violently reactive");
        strcpy(containment, "a need to avoid spillages from entering drains or water courses");
        if(color == 1)
            strcpy(protection,"breathing apparatus, protective gloves for fire only");
        else strcpy(protection, "full protective clothing must be worn");
    }
    printf("***Emergency action advice***\n");
    printf("Material: %s\n", material);
    printf("Reactivity: %s\n" , reactivity);
    printf("Protection: %s\n", protection);
    printf("Containment: %s\n", containment);
    printf("Evacuation: %s\n", evacuation);
    printf("*****************************\n");
}
