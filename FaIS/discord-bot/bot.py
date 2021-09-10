import os
import random
import copy

import discord
from discord.ext import commands
from dotenv import load_dotenv

load_dotenv()
TOKEN = os.getenv('DISCORD_TOKEN')
GUILD = os.getenv('DISCORD_GUILD')

intents = discord.Intents(guilds=True, members=True, messages=True)
bot = commands.Bot(command_prefix='!', intents=intents)

def find_member_by_mention(mention):
    mention_id = copy.copy(mention)
    guild = discord.utils.find(lambda g: g.name == GUILD, bot.guilds)
    if mention_id[:2] == '<@' and mention_id[-1] == '>':
        mention_id = mention_id[2:-1]
        if mention_id[0] == '!':
            mention_id = mention_id[1:]
    member = discord.utils.find(lambda m: m.id == int(mention_id), guild.members)
    return member

@bot.event
async def on_ready():
    guild = discord.utils.get(bot.guilds, name=GUILD)
    print(f'{bot.user} has connected to the guild {guild.name} (id: {guild.id})')

@bot.command(name='info')
async def info(ctx):
    await ctx.send('I am an experimental bot created by SILVER.')

@bot.command(name='insult', help='I am going to hurt your feelings if SILVER says so!')
async def insult(ctx, mention=''):
    if mention == '':
        await ctx.send('You should mention someone')
        return

    try:
        member = find_member_by_mention(mention)
    except:
        await ctx.send('There is no such member, SILVER. You should put that trash\'s name correctly.')
        return

    if not member:
        await ctx.send('There is no such member, SILVER. You should put that trash\'s name correctly.')
        return
    elif member.name == 'SILVER':
        await ctx.send('SILVER has already thought of such edge cases, doofus!')
        return

    insults = ['You are really a piece of shit... You know that, right?!',
            'Stop it! Get some HELP!',
            'You mad bro? Guess what? NOBODY EVER CARES.',
            'Jesus! Your IQ level should be even less than the respect that nobody shows you...']

    await ctx.send('%s, %s' % (mention, random.choice(insults)))

@bot.command(name='respect', help='I am going to respect those who deserve it.')
async def respect(ctx, mention=''):
    if mention == '':
        await ctx.send('You should mention someone')
        return

    try:
        member = find_member_by_mention(mention)
    except:
        await ctx.send('There is no such member, SILVER. You should put that trash\'s name correctly.')
        return

    if not member:
        await ctx.send('There is no such member, SILVER. You should be more careful next time to not disrespect someone which is a man of honor.')
        return
    elif member.name == 'SILVER':
        await ctx.send('SILVER doesn\'t need your respect but thanks anyways...')
        return

    respects = ['Keep doing whatever you are doing. You are great at it!',
            'Our society needs more people like you!',
            'Good job and well done!',
            'I\'ll spam F for you! F... F... F...']

    await ctx.send('%s, %s' % (mention, random.choice(respects)))

@bot.command(name='what2play', help='I will help you to decide what to play now')
async def what2play(ctx, *argv):
    if len(argv) < 2:
        await ctx.send('Give me more options!')
        return
    await ctx.send(f'My vote is for {random.choice(argv)}!')

bot.run(TOKEN)
