import numpy as np
import pandas as pd

''' Database structure:

member.id, member.xp, member.loyalty, member.honor, member.iq, member.respect

id      - id of a member
xp      - experience of a member; interactions in the server
loyalty - interactions with the bot
honor   - how well-mannered a member is
iq      - how intelligent a member is
respect - how much others show respect to a member
'''

def get_dashboard(members):
    df = pd.read_csv('db.csv', header=None)
    return df

