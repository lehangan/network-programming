#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#define M 1000

typedef struct Node
{
    int id;
    char firstName[200];
    char lastName[200];
    float p_progress;
    float p_final;
    char point;
    float over;
    struct Node* next;
} Node;

typedef struct subject
{
    char id[10];
    char name[200];
    int p;
    char sem[10];
    int num;
    int id_highest;
    float avg;
    int A;
    int B;
    int C;
    int D;
    int F;
    Node *head;
} subject;

subject s[M];


Node* makeNode(int id, char* first, char* last, float p_pro, float p_final, char diem, float p_overrall)
{
    Node* p = (Node*)malloc(sizeof(Node));
    p->next = NULL;
    p->id = id;
    strcpy(p->firstName,first);
    strcpy(p->lastName, last);
    p->p_progress = p_pro;
    p->p_final = p_final;
    p->point = diem;
    p->over = p_overrall;
    return p;
}
Node *addToList(Node* h, int id, char* first, char* last, float p_pro, float p_final, char diem, float p_overrall)
{
    if( h == NULL ) return makeNode(id, first, last, p_pro, p_final, diem, p_overrall);
    h->next = addToList(h->next,id, first, last, p_pro, p_final, diem, p_overrall);
    return h;
}

Node *removeNodeRecursive(Node* h, int stu_id)
{
    if( h == NULL ) return NULL;
    if( h->id == stu_id)
    {
        Node* tmp = h;
        h = h->next;
        free(tmp);// thu hoi vung nho
        return h;
    }
    h->next = removeNodeRecursive(h->next, stu_id);
    return h;
}
Node *findStudent(Node* h, int stu_id)
{
    if( h == NULL ) return NULL;
    Node* tmp = h;
    for( Node* p = tmp ; p!= NULL; p=p->next)
    {
        if( p->id == stu_id ) return p;
    }
    return NULL;
}

int num_sub = 0;
void addScoreBoard(char *id,char* name, int progress, char* sem_id, int number )
{
    num_sub++;
    strcpy(s[num_sub].id,id);
    strcpy(s[num_sub].name,name);
    strcpy(s[num_sub].sem,sem_id);
    s[num_sub].p = progress;
    s[num_sub].num = number;
    s[num_sub].head = NULL;
    s[num_sub].A = 0;
    s[num_sub].B = 0;
    s[num_sub].B = 0;
    s[num_sub].D = 0;
    s[num_sub].F = 0;
    s[num_sub].avg = 0;
    s[num_sub].id_highest = 0;
}

char calPoint(float rounded)
{
    //float cal_point = (p_pro*f+(100-f)*p_final)/100;
    //float rounded = ((int)(cal_point * 10 + 0.5)) / 10.0;
    if(rounded >= 0 && rounded< 4.0 ) return 'F';
    else if(rounded >= 4.0 && rounded <5.5) return 'D';
    else if(rounded >=5.5 && rounded <7.0) return 'C';
    else if(rounded >= 7.0 && rounded <8.5 ) return 'B';
    else return 'A';
}

void addScore(char* sem_id, char* sub_id, int stu_id, char* first, char* last, float p_pro, float p_final )
{
    for( int i = 1 ; i<= num_sub; i++)
    {
        if( !strcmp(s[i].sem, sem_id) && !strcmp(s[i].id, sub_id))
        {
            int F = s[i].p;
            float pro = p_pro;
            float fin = p_final;
            float cal_point = (p_pro*F+(100-F)*p_final)/100;
            float rounded = ((int)(cal_point * 10 + 0.5)) / 10.0;
            char diem = calPoint(rounded);
            s[i].head = addToList(s[i].head, stu_id, first, last, p_pro, p_final, diem, cal_point);
        }
    }
}
void removeScore(char* sem_id, char* sub_id, int stu_id)
{
    for( int i = 1 ; i<= num_sub; i++)
    {
        if( !strcmp(s[i].sem, sem_id) && !strcmp(s[i].id, sub_id))
        {
            s[i].head = removeNodeRecursive(s[i].head, stu_id);
        }
    }
}
Node* searchScore(char* sem_id, char* sub_id, int stu_id)
{
    for( int i = 1 ; i<= num_sub; i++)
    {
        if( !strcmp(s[i].sem, sem_id) && !strcmp(s[i].id, sub_id))
        {
            Node* tmp = findStudent(s[i].head, stu_id);
            return tmp;
        }
    }
    return NULL;
}

void reportScore(char* sem_id, char* sub_id)
{
    char filename[200];
    for( int i = 1 ; i<= num_sub; i++)
    {
        if( !strcmp(s[i].sem, sem_id) && !strcmp(s[i].id, sub_id))
        {
            char filename[200];
            snprintf(filename, sizeof(filename), "%s_%srp.txt", s[i].id, s[i].sem);
            //FILE *file = fopen(filename, "w");
            FILE *file = fopen(filename, "w+");
            if (file == NULL)
            {
                printf("Failed to open file %s for writing.\n", filename);
            }
            Node* tmp = s[i].head;
            int high = tmp->over;
            int high_id = tmp->id;
            int low = tmp->over;
            int low_id = tmp->id;
            float tong = 0.0;
            int number_stu = 0;
            if( tmp!= NULL)
            {
                for( Node* p = tmp ; p!=NULL;  p=p->next)
                {
                    tong+= p->over;
                    number_stu++;
                    if(p->over>= high)
                    {
                        high = p->over;
                        high_id = p->id;
                    }
                    if(p->over <= low)
                    {
                        low = p->over;
                        low_id = p->id;
                    }
                    if( p->point == 'A') s[i].A++;
                    if( p->point == 'B') s[i].B++;
                    if( p->point == 'C') s[i].C++;
                    if( p->point == 'D') s[i].D++;
                    if( p->point == 'F') s[i].F++;
                }
            }
            s[i].avg = (float) tong/number_stu;
            fprintf(file, "%s_%s_rp.txt", s[i].id, s[i].sem);
            fprintf(file, "\n\n");
            Node *high_stu = searchScore(s[i].sem, s[i].id, high_id);
            Node *low_stu = searchScore(s[i].sem, s[i].id, low_id);
            fprintf(file, "The student with the highest mark is: %s %s\n", high_stu->lastName, high_stu->firstName);
            fprintf(file, "The student with the highest mark is: %s %s\n", low_stu->lastName, low_stu->firstName);
            fprintf(file, "The average mark is: %.2f\n\n", s[i].avg);
            fprintf(file, "A histogram of the subject %s is: \n", s[i].id);
            fprintf(file, "A:");
            int nA = s[i].A;
            for( int i = 1 ; i<= nA; i++)
            {
                fprintf(file, "*");
            }
            fprintf(file, "\n");
            int nB = s[i].B;
            fprintf(file, "B:");
            for( int i = 1 ; i<= nB; i++)
                fprintf(file, "*");
            fprintf(file, "\n");
            fprintf(file, "C:");
            int nC = s[i].C;
            for( int i = 1 ; i<= nC; i++)
                fprintf(file, "*");
            fprintf(file, "\n");
            fprintf(file, "D:");
            int nD = s[i].D;
            for( int i = 1 ; i<= nD; i++)
                fprintf(file, "*");
            fprintf(file, "\n");
            fprintf(file, "F:");
            int nF = s[i].F;
            for( int i = 1 ; i<= nF; i++)
                fprintf(file, "*");
            fprintf(file, "\n");
            fclose(file);
        }
    }
}
void displayScore(char* sem_id, char* sub_id)
{
    char filename[200];
    for( int i = 1 ; i<= num_sub; i++)
    {
        if( !strcmp(s[i].sem, sem_id) && !strcmp(s[i].id, sub_id))
        {
            char filename[200];
            snprintf(filename, sizeof(filename), "%s_%s.txt", s[i].id, s[i].sem);
            FILE *file1 = fopen(filename, "w+");
            if (file1 == NULL)
            {
                printf("Failed to open file %s for writing.\n", filename);
            }
            fprintf(file1, "SubjectID|%s\n", s[i].id);
            fprintf(file1, "Subject|%s\n", s[i].name);
            fprintf(file1, "F|%d|%d\n", s[i].p, (100 - s[i].p));
            fprintf(file1, "Semester|%s\n", s[i].sem);
            fprintf(file1, "StudentCount|%d\n", s[i].num);
            Node* tmp = s[i].head;
            if( tmp!= NULL)
            {
                for( Node* p = tmp ; p!=NULL;  p=p->next)
                {
                    fprintf(file1, "S|%d|%-20s|%-10s|% -6.1f|% -6.1f| %-2c|\n", p->id, p->lastName, p->firstName, p->p_progress,p->p_final, p->point);
                }
            }
            fclose(file1);
        }
    }
}
void menu()
{
    printf("Learning Management System\n");
    printf("-------------------------------------\n");
    printf("    1.    Add a new score board\n");
    printf("    2.    Add score\n");
    printf("    3.    Remove score\n");
    printf("    4.    Search score\n");
    printf("    5.    Display score board and score report\n");
    printf("Your choice (1-5, other to quit):");
}

char sub_id[10];
char *sub_name = NULL;
int percentage;
char sem[10];
int num;

char sub_id[10];
char sem[10];
int stu_id;
char fullName[200];
char firstName[200];
char lastName[200];
float p_pro;
float p_final;

void function1()
{
    printf("Subject ID:");
    scanf("%s", sub_id);
    printf("Semester:");
    scanf("%s", sem);
    printf("Student Count:");
    scanf("%d", &num);
    char filename[200];
    snprintf(filename, sizeof(filename), "%s_%s.txt", sub_id, sem);

    // Open the file for reading (you can use fopen for other file operations)
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Failed to open file or the file did not exits %s\n", filename);
    }
    char line[M];

    // Read and process each line in the file
    while (fgets(line, sizeof(line), file))
    {
        // Tokenize the line using '|' as the delimiter
        char *token = strtok(line, "|");
        if (token)
        {
            // Check the first token to determine the type of data
            if (strcmp(token, "Subject") == 0)
            {
                token = strtok(NULL, "|");
                sub_name = strdup(token);
            }
            else if (strcmp(token, "F") == 0)
            {
                percentage = atoi(strtok(NULL, "|"));
            }
            else if (strcmp(token, "StudentCount") == 0)
            {
                num = atoi(strtok(NULL, "|"));
            }
        }
    }

    addScoreBoard(sub_id, sub_name, percentage, sem, num);
    fclose(file);
}
void xuliten(char*c)
{
    int n = strlen(c);
    int danhdau = n;
    if(c[n] != '\0') c[n] = '\0';
    for( int i = n-1 ; i>=0 ; i--)
    {
        if(c[i] != ' ' )
        {
            danhdau = i;
            break;
        }
    }
    c[danhdau+1] = '\0';
}
void function2()
{
    printf("A subject you want to add score of student:\n");
    printf("Subject ID:");
    scanf("%s", sub_id);
    printf("Semester:");
    scanf("%s", sem);
    char filename[200];
    snprintf(filename, sizeof(filename), "%s_%s.txt", sub_id, sem);

    // Open the file for reading (you can use fopen for other file operations)
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Failed to open file or the file did not exits %s\n", filename);
    }
    char line[M];

    // Read and process each line in the file
    while (fgets(line, sizeof(line), file))
    {
        // Tokenize the line using '|' as the delimiter
        char *token = strtok(line, "|");
        if (token)
        {
            if (strcmp(token, "S") == 0)
            {

                // Parse the student data
                stu_id = atoi(strtok(NULL, "|"));
                strcpy(lastName, strtok(NULL, "|"));
                strcpy(firstName, strtok(NULL, "|"));
                xuliten(firstName);
                xuliten(lastName);
                p_pro = atof(strtok(NULL, "|"));
                p_final = atof(strtok(NULL, "|"));
                addScore(sem, sub_id,stu_id, firstName, lastName, p_pro, p_final);
            }

        }
    }

    fclose(file);

}
void function3()
{
    printf("A subject you want to delete the grade :\n");
    printf("Subject ID:");
    scanf("%s", sub_id);
    printf("Semester:");
    scanf("%s", sem);
    printf("Student Information:\n");
    printf("StudentID:");
    scanf("%d", &stu_id);
    removeScore(sem, sub_id, stu_id);
}

void function4()
{
    printf("A subject you want to find the grade :\n");
    printf("Subject ID:");
    scanf("%s", sub_id);
    printf("Semester:");
    scanf("%s", sem);
    printf("Student Information:\n");
    printf("StudentID:");
    scanf("%d", &stu_id);
    Node* p = searchScore(sem, sub_id, stu_id);
    if( p == NULL )
    {
        printf("NOT FIND THIS STUDENT\n");
    }
    else
    {
        printf("S|%d|%-20s|%-10s|% -6.1f|% -6.1f| %-2c|\n", p->id, p->lastName, p->firstName, p->p_progress,p->p_final, p->point);
    }
}
void function5()
{
    char id[10];
    char sem[10];
    printf("A subject you want to show:\n");
    printf("Subject ID:");
    scanf("%s", id);
    printf("Semester:");
    scanf("%s", sem);
    displayScore(sem,id);
    reportScore(sem, id);
}

int main()
{
    int choice;
    while(1)
    {
        menu();
        scanf("%d", &choice);
        if( choice == 1 )
        {
            char c;
            do
            {
                function1();
                printf("Do you want to continue or not?");
                fflush(stdin);
                scanf("%c", &c);
            }
            while(c == 'y' || c == 'Y');
        }
        else if(choice == 2)
        {
            char c;
            do
            {
                function2();
                printf("Do you want to continue or not?");
                fflush(stdin);
                scanf("%c", &c);
            }
            while(c == 'y' || c == 'Y');
        }
        else if(choice == 3)
        {
            char c;
            do
            {
                function3();
                printf("Do you want to continue or not?");
                fflush(stdin);
                scanf("%c", &c);
            }
            while(c == 'y' || c == 'Y');
        }
        else if(choice == 4)
        {
            char c;
            do
            {
                function4();
                printf("Do you want to continue or not?");
                fflush(stdin);
                scanf("%c", &c);
            }
            while(c == 'y' || c == 'Y');
        }
        else if(choice == 5)
        {
            char c;
            do
            {
                function5();
                printf("Do you want to continue or not?");
                fflush(stdin);
                scanf("%c", &c);
            }
            while(c == 'y' || c == 'Y');
        }

        else break;
    }
    return 0;
}
